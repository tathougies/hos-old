TOPDIR=../../..

include $(TOPDIR)/build/settings.mk
include $(TOPDIR)/build/common.mk

INCLUDES=-I$(BOOTCOMMONDIR) -I$(BOOTTOPDIR)/src

COMMON_TARGET_OBJS=$(addprefix $(BUILDDIR)/boot/,terminal.o)

.PHONY: common

$(BUILDDIR)/boot/%.o: %.c
	$(CC32) $(CFLAGS) $(INCLUDES) -c $< -o $@ -nostdlib -ffreestanding

common: $(COMMON_TARGET_OBJS)