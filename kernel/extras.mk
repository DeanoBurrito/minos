#useful makefile extras, not related to actually building the kernel

OVMF_DIR = /usr/share/ovmf/OVMF.fd
QEMU_FLAGS = -machine q35 -smp cores=4 -serial mon:stdio -m 256M -drive if=pflash,format=raw,readonly,file=$(OVMF_DIR) -cdrom $(TARGET).iso

run: iso
	@qemu-system-x86_64 $(QEMU_FLAGS)

debug: iso
	@qemu-system-x86_64 $(QEMU_FLAGS) -s -S

ifeq ($(BOOT_PLATFORM), multiboot)
iso: all
	@cp $(TARGET) $(BOOT_PLATFORM_DIR)/build/kernel.elf
	@cd $(BOOT_PLATFORM_DIR); make mb-pack
	@cp $(BOOT_PLATFORM_DIR)/build/kernel-patched.elf $(BUILD_DIR)/kernel-patched.elf
else
iso: iso-img
	@echo CREATING ISO
	@mkdir -p $(BUILD_DIR)/iso
	@cp $(TARGET).img $(BUILD_DIR)/iso
	@cd $(BUILD_DIR); xorriso -as mkisofs -R -f -e $(TARGET_NAME).img -no-emul-boot -o $(TARGET_NAME).iso iso
endif
	
iso-img: all
	@echo Getting bootloader file
	@cd $(BOOT_PLATFORM_DIR); make all;
	@cp $(BOOTLOADER_FILE) $(BUILD_DIR)/BOOTX64.EFI
	@dd if=/dev/zero of=$(TARGET).img bs=1K count=2880
	@mformat -i $(TARGET).img -f 2880 ::
	@mmd -i $(TARGET).img ::/EFI
	@mmd -i $(TARGET).img ::/EFI/BOOT
	@mcopy -i $(TARGET).img $(BUILD_DIR)/BOOTX64.EFI ::/EFI/BOOT
	@mcopy -i $(TARGET).img $(TARGET) ::
	@mcopy -i $(TARGET).img $(ASSETS_DIR) ::/assets
	@rm $(BUILD_DIR)/BOOTX64.EFI
