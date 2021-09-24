#useful makefile extras, not related to actually building the kernel

OVMF_DIR = /usr/share/ovmf/OVMF.fd
QEMU_FLAGS = -machine q35 -smp cores=4 -serial mon:stdio -m 256M -drive if=pflash,format=raw,readonly,file=$(OVMF_DIR) -cdrom $(BUILD_DIR)/minos-$(ARCH)-$(BOOT_PLATFORM).iso

run: iso
	@qemu-system-x86_64 $(QEMU_FLAGS)

debug: iso
	@qemu-system-x86_64 $(QEMU_FLAGS) -s -S

# Interface for bootloader makefiles: pack command is used to assemble final iso
# input is bootloader-dir/build/kernel.elf: this is the compiled kernel file, ready to go.
# output is expected at bootloader-dir/build/packed.iso
iso: all
	@mkdir -p $(BOOT_PLATFORM_DIR)/$(BUILD_DIR)
	@cp $(TARGET) $(BOOT_PLATFORM_DIR)/$(BUILD_DIR)/kernel.elf
	@cd $(BOOT_PLATFORM_DIR); make pack;
	@cp $(BOOT_PLATFORM_DIR)/$(BUILD_DIR)/packed.iso $(BUILD_DIR)/minos-$(ARCH)-$(BOOT_PLATFORM).iso
