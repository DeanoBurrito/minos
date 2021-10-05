#external project references
GNUEFI_DIR = ../../../gnu-efi
OVMF_DIR = /usr/share/ovmf/OVMF.fd

#internal project settings
TARGET_NAME = booty
BUILD_DIR = build
TARGET = $(BUILD_DIR)/$(TARGET_NAME)
C_SRCS = $(shell find -name "*.c")
CRT0 = $(GNUEFI_DIR)/x86_64/gnuefi/crt0-efi-x86_64.o

#flags
CC_INC_DIRS = -I $(GNUEFI_DIR)/inc
CC_FLAGS = -fPIC -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args -Wall
LIBS_DIRS = -L $(GNUEFI_DIR)/x86_64/lib -L $(GNUEFI_DIR)/x86_64/gnuefi
LD_LIBS = -lgnuefi -lefi
LD_FLAGS = -shared -Bsymbolic
LD_SCRIPT = $(GNUEFI_DIR)/gnuefi/elf_x86_64_efi.lds

#toolchain selection
OBJCOPY = objcopy
CC = gcc
LD = ld

#auto populated vars
CC_OBJS = $(patsubst %.c, $(BUILD_DIR)/%.o, $(C_SRCS))

.PHONY: all clean iso run pack

all: $(TARGET).efi

clean:
	@echo DELETING $(BUILD_DIR) ...
	@rm -r $(BUILD_DIR)

iso: $(TARGET).img
	@echo MAKING ISO
	@mkdir -p $(BUILD_DIR)/iso
	@cp $(TARGET).img $(BUILD_DIR)/iso
	@cd $(BUILD_DIR); xorriso -as mkisofs -R -f -e $(TARGET_NAME).img -no-emul-boot -o $(TARGET_NAME).iso iso

run: iso
	@qemu-system-x86_64 -drive if=pflash,format=raw,readonly,file=$(OVMF_DIR) -cdrom $(TARGET).iso

pack: all
	@echo Packing kernel with x86_64-uefi bootloader
	@cp $(TARGET).efi $(BUILD_DIR)/BOOTX64.EFI
	@dd if=/dev/zero of=$(BUILD_DIR)/kernel-patched.img bs=1k count=2880
	@mformat -i $(BUILD_DIR)/kernel-patched.img -f 2880 ::
	@mmd -i $(BUILD_DIR)/kernel-patched.img ::/EFI
	@mmd -i $(BUILD_DIR)/kernel-patched.img ::/EFI/BOOT
	@mcopy -i $(BUILD_DIR)/kernel-patched.img $(BUILD_DIR)/BOOTX64.EFI ::EFI/BOOT/
	@mcopy -i $(BUILD_DIR)/kernel-patched.img $(BUILD_DIR)/kernel.elf ::
	@rm $(BUILD_DIR)/BOOTX64.EFI
	@mkdir -p $(BUILD_DIR)/iso
	@cp $(BUILD_DIR)/kernel-patched.img $(BUILD_DIR)/iso
	@cd $(BUILD_DIR); xorriso -as mkisofs -R -f -e kernel-patched.img -no-emul-boot -o packed.iso iso

$(TARGET).so: $(CC_OBJS)
	@echo LINKING $<
	@$(LD) $(LD_FLAGS) $(LIBS_DIRS) -T $(LD_SCRIPT) $(CRT0) $(CC_OBJS) -o $@ $(LD_LIBS)

$(BUILD_DIR)/%.o: %.c
	@echo COMPILING $<
	@mkdir -p $(BUILD_DIR)
	@$(CC) $(CC_INC_DIRS) $(CC_FLAGS) -c $< -o $@

%.efi: %.so
	@echo CONVERTING $< to $@
	@$(OBJCOPY) -j .text -j .sdata -j .data -j .dynamic -j .dynsym  -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --target efi-app-x86_64 --subsystem=10 $< $@