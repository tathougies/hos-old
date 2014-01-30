TOPDIR=../..

include $(TOPDIR)/build/settings.mk
include $(TOPDIR)/build/common.mk

OBJS=$(addprefix $(BUILDDIR)/kernel/,init.o)

.PHONY: all arch

all: arch $(BUILDDIR)/kernel/kernel.bin

arch:
	mkdir -p $(BUILDDIR)/kernel/arch
	$(MAKE) -C $(ARCH)

$(BUILDDIR)/kernel/%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

$(BUILDDIR)/kernel/kernel.bin: $(BUILDDIR)/kernel/arch.a $(OBJS)
	$(LD) -o $(BUILDDIR)/kernel/kernel.bin -T$(ARCH)/linker.ld $(OBJS) $(BUILDDIR)/kernel/arch.a
