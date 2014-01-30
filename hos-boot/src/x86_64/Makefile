TOPDIR=../../..

include $(TOPDIR)/build/settings.mk
include $(TOPDIR)/build/common.mk

INCLUDES=-I$(BOOTCOMMONDIR) -I$(BOOTTOPDIR)/src

I386_OBJS=$(addprefix $(BUILDDIR)/boot/,terminal.o)
COMMON_OBJS=$(addprefix $(BUILDDIR)/boot/,string.o math.o stdlib.o error.o options.o)
OBJS=$(addprefix $(BUILDDIR)/boot/,boot.o bootstrap.o) $(I386_OBJS) $(COMMON_OBJS)

.PHONY: all all-common

all: all-common $(BUILDDIR)/boot $(BUILDDIR)/boot/hos-boot.bin

$(BUILDDIR)/boot/boot.o: boot.S
	$(CC32) $(CFLAGS) $(INCLUDES) -c boot.S -o $(BUILDDIR)/boot/boot.o -ffreestanding

$(BUILDDIR)/boot/bootstrap.o: bootstrap.c $(BOOTTOPDIR)/src/i386/terminal.h $(BOOTTOPDIR)/src/i386/multiboot.h
	$(CC32) $(CFLAGS) $(INCLUDES) -c bootstrap.c -o $(BUILDDIR)/boot/bootstrap.o -ffreestanding -nostdlib

$(BUILDDIR)/boot/hos-boot.bin: $(OBJS)
	$(CC) -Wl,-melf_i386 -o $(BUILDDIR)/boot/hos-boot.bin -T link.ld $(OBJS) -nostdlib

$(OBJS): | $(BUILDDIR)/boot

$(BUILDDIR)/boot:
	mkdir -p $(BUILDDIR)/boot

all-common:
	make -C $(BOOTTOPDIR)/src/i386 common
	make -C $(BOOTCOMMONDIR)
