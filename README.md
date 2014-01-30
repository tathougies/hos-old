HOS: The Haskell Operating System
=================================

This is a project to build an operating system completely out of Haskell. Wherever possible we use
Haskell, where "normal" operating systems may use C or Assembler.

We use GHC to compile Haskell, which means that our compiled Haskell does depend on a C runtime
library. This dependency on C could probably be gotten rid of if we heavily modified GHC, but this
is beyond the scope of the project. For our kernel, we compile GHC's runtime system, and define all
the C standard library functionality it needs, but in kernel land. In the initial development phase,
I plan on using a lot of C shims to get the rts working. As more things start to work, I'll slowly
try removing C bits.

What works
----------

Right now, you can build a bootable x86-64 ISO by running make in the downloaded directory. You will
probably need to modify `build/settings.mk`. An i386 port is planned in the future, and this
shouldn't be difficult once x86-64 is up and working.

What doesn't work
-----------------

We haven't yet written anything in Haskell. We currently have a bootloader which can load
stripped-down ELF64 files into the higher half of memory (`0xffff800000000000`), and then jump to
the entry point.

We still need to compile GHC's RTS in a safe way, and get rid of all the functionality that we don't
need. Any suggestions on how to do that are very much appreciated.

Steps going forward
--------------------

1. Compile the Haskell RTS in kernel mode
2. Write enough boiler plate C so that it compiles
3. Write our first Haskell `main` function that runs in kernel land

Copyright Notice
----------------

The code in this repository is Copyright (c) 2014 Travis Athougies. All Rights Reserved. Use of this
code without the direct written permission of the author is prohibited, but it may be examined for
educational purposes only.
