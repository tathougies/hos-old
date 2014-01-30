TOPDIR=.

include $(TOPDIR)/build/settings.mk
include $(TOPDIR)/build/common.mk

.PHONY: all hos-boot kernel

all: $(BUILDDIR)/hos.iso

$(BUILDDIR)/hos.iso: hos-boot kernel
	mkdir -p $(ISODIR)/boot
	cp $(BUILDDIR)/boot/hos-boot.bin $(ISODIR)/boot
	cp $(BUILDDIR)/kernel/kernel.bin $(ISODIR)/boot/Interlude
	cp -R $(BOOTSRCDIR)/grub $(ISODIR)/boot/
	grub-mkrescue -o $(BUILDDIR)/hos.iso $(ISODIR)

hos-boot:
	mkdir -p $(BUILDDIR)
	$(MAKE) -C hos-boot

kernel:
	mkdir -p $(BUILDDIR)
	$(MAKE) -C kernel

clean:
	rm -r $(BUILDDIR)