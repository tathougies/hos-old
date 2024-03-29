/* -----------------------------------------------------------------------------
 *
 * (c) The GHC Team 1998-2008
 *
 * Weak pointers and weak-like things in the GC
 *
 * Documentation on the architecture of the Garbage Collector can be
 * found in the online commentary:
 * 
 *   http://hackage.haskell.org/trac/ghc/wiki/Commentary/Rts/Storage/GC
 *
 * ---------------------------------------------------------------------------*/

#include "PosixSource.h"
#include "Rts.h"

#include "MarkWeak.h"
#include "GC.h"
#include "GCThread.h"
#include "GCTDecl.h"
#include "Evac.h"
#include "Trace.h"
#include "Schedule.h"
#include "Weak.h"
#include "Storage.h"
#include "Threads.h"

/* -----------------------------------------------------------------------------
   Weak Pointers

   traverse_weak_ptr_list is called possibly many times during garbage
   collection.  It returns a flag indicating whether it did any work
   (i.e. called evacuate on any live pointers).

   Invariant: traverse_weak_ptr_list is called when the heap is in an
   idempotent state.  That means that there are no pending
   evacuate/scavenge operations.  This invariant helps the weak
   pointer code decide which weak pointers are dead - if there are no
   new live weak pointers, then all the currently unreachable ones are
   dead.

   For generational GC: we just don't try to finalize weak pointers in
   older generations than the one we're collecting.  This could
   probably be optimised by keeping per-generation lists of weak
   pointers, but for a few weak pointers this scheme will work.

   There are three distinct stages to processing weak pointers:

   - weak_stage == WeakPtrs

     We process all the weak pointers whos keys are alive (evacuate
     their values and finalizers), and repeat until we can find no new
     live keys.  If no live keys are found in this pass, then we
     evacuate the finalizers of all the dead weak pointers in order to
     run them.

   - weak_stage == WeakThreads

     Now, we discover which *threads* are still alive.  Pointers to
     threads from the all_threads and main thread lists are the
     weakest of all: a pointers from the finalizer of a dead weak
     pointer can keep a thread alive.  Any threads found to be unreachable
     are evacuated and placed on the resurrected_threads list so we 
     can send them a signal later.

   - weak_stage == WeakDone

     No more evacuation is done.

   -------------------------------------------------------------------------- */

/* Which stage of processing various kinds of weak pointer are we at?
 * (see traverse_weak_ptr_list() below for discussion).
 */
typedef enum { WeakPtrs, WeakThreads, WeakDone } WeakStage;
static WeakStage weak_stage;

/* Weak pointers
 */
StgWeak *old_weak_ptr_list; // also pending finaliser list
StgWeak *weak_ptr_list_tail;

// List of threads found to be unreachable
StgTSO *resurrected_threads;

static void resurrectUnreachableThreads (generation *gen);
static rtsBool tidyThreadList (generation *gen);

void
initWeakForGC(void)
{
    old_weak_ptr_list = weak_ptr_list;
    weak_ptr_list = NULL;
    weak_ptr_list_tail = NULL;
    weak_stage = WeakPtrs;
    resurrected_threads = END_TSO_QUEUE;
}

rtsBool 
traverseWeakPtrList(void)
{
  StgWeak *w, **last_w, *next_w;
  StgClosure *new;
  rtsBool flag = rtsFalse;
  const StgInfoTable *info;

  switch (weak_stage) {

  case WeakDone:
      return rtsFalse;

  case WeakPtrs:
      /* doesn't matter where we evacuate values/finalizers to, since
       * these pointers are treated as roots (iff the keys are alive).
       */
      gct->evac_gen_no = 0;
      
      last_w = &old_weak_ptr_list;
      for (w = old_weak_ptr_list; w != NULL; w = next_w) {
	  
	  /* There might be a DEAD_WEAK on the list if finalizeWeak# was
	   * called on a live weak pointer object.  Just remove it.
	   */
	  if (w->header.info == &stg_DEAD_WEAK_info) {
	      next_w = ((StgDeadWeak *)w)->link;
	      *last_w = next_w;
	      continue;
	  }
	  
          info = get_itbl(w);
	  switch (info->type) {

	  case WEAK:
	      /* Now, check whether the key is reachable.
	       */
	      new = isAlive(w->key);
	      if (new != NULL) {
		  w->key = new;
		  // evacuate the value and finalizer 
		  evacuate(&w->value);
		  evacuate(&w->finalizer);
		  // remove this weak ptr from the old_weak_ptr list 
		  *last_w = w->link;
                  next_w  = w->link;

                  // and put it on the new weak ptr list.
                  // NB. we must retain the order of the weak_ptr_list (#7160)
                  if (weak_ptr_list == NULL) {
                      weak_ptr_list = w;
                  } else {
                      weak_ptr_list_tail->link = w;
                  }
                  weak_ptr_list_tail = w;
                  w->link = NULL;
                  flag = rtsTrue;

		  debugTrace(DEBUG_weak, 
			     "weak pointer still alive at %p -> %p",
			     w, w->key);
		  continue;
	      }
	      else {
		  last_w = &(w->link);
		  next_w = w->link;
		  continue;
	      }

	  default:
	      barf("traverseWeakPtrList: not WEAK");
	  }
      }
      
      /* If we didn't make any changes, then we can go round and kill all
       * the dead weak pointers.  The old_weak_ptr list is used as a list
       * of pending finalizers later on.
       */
      if (flag == rtsFalse) {
	  for (w = old_weak_ptr_list; w; w = w->link) {
	      evacuate(&w->finalizer);
	  }

	  // Next, move to the WeakThreads stage after fully
	  // scavenging the finalizers we've just evacuated.
	  weak_stage = WeakThreads;
      }

      return rtsTrue;

  case WeakThreads:
      /* Now deal with the step->threads lists, which behave somewhat like
       * the weak ptr list.  If we discover any threads that are about to
       * become garbage, we wake them up and administer an exception.
       */
  {
      nat g;
	  
      // Traverse thread lists for generations we collected...
//      ToDo when we have one gen per capability:
//      for (n = 0; n < n_capabilities; n++) {
//          if (tidyThreadList(&nurseries[n])) {
//              flag = rtsTrue;
//          }
//      }              
      for (g = 0; g <= N; g++) {
          if (tidyThreadList(&generations[g])) {
              flag = rtsTrue;
          }
      }

      /* If we evacuated any threads, we need to go back to the scavenger.
       */
      if (flag) return rtsTrue;

      /* And resurrect any threads which were about to become garbage.
       */
      {
          nat g;
          for (g = 0; g <= N; g++) {
              resurrectUnreachableThreads(&generations[g]);
          }
      }
        
      weak_stage = WeakDone;  // *now* we're done,
      return rtsTrue;         // but one more round of scavenging, please
  }
      
  default:
      barf("traverse_weak_ptr_list");
      return rtsTrue;
  }
}
  
  static void resurrectUnreachableThreads (generation *gen)
{
    StgTSO *t, *tmp, *next;

    for (t = gen->old_threads; t != END_TSO_QUEUE; t = next) {
        next = t->global_link;
        
        // ThreadFinished and ThreadComplete: we have to keep
        // these on the all_threads list until they
        // become garbage, because they might get
        // pending exceptions.
        switch (t->what_next) {
        case ThreadKilled:
        case ThreadComplete:
            continue;
        default:
            tmp = t;
            evacuate((StgClosure **)&tmp);
            tmp->global_link = resurrected_threads;
            resurrected_threads = tmp;
        }
    }
}

static rtsBool tidyThreadList (generation *gen)
{
    StgTSO *t, *tmp, *next, **prev;
    rtsBool flag = rtsFalse;

    prev = &gen->old_threads;

    for (t = gen->old_threads; t != END_TSO_QUEUE; t = next) {
	      
        tmp = (StgTSO *)isAlive((StgClosure *)t);
	
        if (tmp != NULL) {
            t = tmp;
        }
        
        ASSERT(get_itbl(t)->type == TSO);
        next = t->global_link;
        
        // if the thread is not masking exceptions but there are
        // pending exceptions on its queue, then something has gone
        // wrong.  However, pending exceptions are OK if there is an
        // FFI call.
        ASSERT(t->blocked_exceptions == END_BLOCKED_EXCEPTIONS_QUEUE
               || t->why_blocked == BlockedOnCCall
               || t->why_blocked == BlockedOnCCall_Interruptible
               || (t->flags & TSO_BLOCKEX));
        
        if (tmp == NULL) {
            // not alive (yet): leave this thread on the
            // old_all_threads list.
            prev = &(t->global_link);
        } 
        else {
            // alive
            *prev = next;
            
            // move this thread onto the correct threads list.
            generation *new_gen;
            new_gen = Bdescr((P_)t)->gen;
            t->global_link = new_gen->threads;
            new_gen->threads  = t;
        }
    }

    return flag;
}

/* -----------------------------------------------------------------------------
   Evacuate every weak pointer object on the weak_ptr_list, and update
   the link fields.

   ToDo: with a lot of weak pointers, this will be expensive.  We
   should have a per-GC weak pointer list, just like threads.
   -------------------------------------------------------------------------- */

void
markWeakPtrList ( void )
{
  StgWeak *w, **last_w;

  last_w = &weak_ptr_list;
  for (w = weak_ptr_list; w; w = w->link) {
      // w might be WEAK, EVACUATED, or DEAD_WEAK (actually CON_STATIC) here

#ifdef DEBUG
      {   // careful to do this assertion only reading the info ptr
          // once, because during parallel GC it might change under our feet.
          const StgInfoTable *info;
          info = w->header.info;
          ASSERT(IS_FORWARDING_PTR(info)
                 || info == &stg_DEAD_WEAK_info 
                 || INFO_PTR_TO_STRUCT(info)->type == WEAK);
      }
#endif

      evacuate((StgClosure **)last_w);
      w = *last_w;
      if (w->header.info == &stg_DEAD_WEAK_info) {
          last_w = &(((StgDeadWeak*)w)->link);
      } else {
          last_w = &(w->link);
      }
  }
}

