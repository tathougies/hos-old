TOPDIR=../../..

include $(TOPDIR)/build/settings.mk
include $(TOPDIR)/build/common.mk

INCLUDES=-I$(BOOTCOMMONDIR) -I$(BOOTTOPDIR)/src

OBJS=$(addprefix $(BUILDDIR)/boot/,string.o math.o stdlib.o error.o options.o)

.PHONY: all

all: $(OBJS)

$(BUILDDIR)/boot/%.o: %.c
	$(BOOTCC) $(CFLAGS) $(INCLUDES) -c $< -o $@ -nostdlib -ffreestanding
