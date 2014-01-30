TOPDIR=../../..

include $(TOPDIR)/build/settings.mk
include $(TOPDIR)/build/common.mk

C64FLAGS=$(CFLAGS) -I$(KERNELTOPDIR)/cbits/$(ARCH)

OBJS=$(addprefix $(BUILDDIR)/kernel/arch/,start.o)

.PHONY: all

all: $(BUILDDIR)/kernel/arch.a

$(BUILDDIR)/kernel/arch/%.o: %.S
	$(CC) -c $< -o $@ $(C64FLAGS)

$(BUILDDIR)/kernel/arch.a: $(OBJS)
	$(AR) rcs $(BUILDDIR)/kernel/arch.a $(OBJS)
	$(RANLIB) $(BUILDDIR)/kernel/arch.a