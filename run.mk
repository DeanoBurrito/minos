#commands for running and debugging images

EFI_FIRMWARE_DIR = /usr/share/ovmf/OVMF.fd
QEMU_FLAGS = -machine q35 -smp cores=4 -serial mon:stdio -m 256M -drive if=pflash,format=raw,readonly,file=$(EFI_FIRMWARE_DIR) -cdrom $(ISO_TARGET)
QEMU_DEBUG_FLAGS = -s -S
QEMU_BIN=qemu-system-$(CPU_ARCH)

#alias to select which build target to use for running/debugging (all/no-apps/min)
run-target: all

run: run-target
	@$(QEMU_BIN) $(QEMU_FLAGS)

debug: run-target
	@$(QEMU_BIN) $(QEMU_FLAGS) $(QEMU_DEBUG_FLAGS)

debug-hang: run-target
	@$(QEMU_BIN) $(QEMU_FLAGS) $(QEMU_DEBUG_FLAGS) -no-reboot -no-shutdown
