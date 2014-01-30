BOOTTOPDIR=$(TOPDIR)/hos-boot/
BOOTSRCDIR=$(BOOTTOPDIR)/src/$(ARCH)
BOOTCOMMONDIR=$(BOOTTOPDIR)/src/common

KERNELTOPDIR=$(TOPDIR)/kernel/

BUILDDIR=$(TOPDIR)/dist/build/$(ARCH)
ISODIR=$(BUILDDIR)/iso

# Compiler and linker stuff
CC=$(XC_TOOLCHAIN)/$(TARGET)-gcc
AR=$(XC_TOOLCHAIN)/$(TARGET)-ar
LD=$(XC_TOOLCHAIN)/$(TARGET)-ld
RANLIB=$(XC_TOOLCHAIN)/$(TARGET)-ranlib
CC32=$(CC) -m32

CFLAGS=-Wall -Werror $(BUILD_CFLAGS) -g -fomit-frame-pointer -fno-exceptions -fno-asynchronous-unwind-tables -fno-unwind-tables

ifeq ($(ARCH),x86_64)
BOOTCC=$(CC32)
else
BOOTCC=$(CC)
endif