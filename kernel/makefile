#external project references
BOOTLOADER_DIR = ../boot
SYSLIB_DIR = ../syslib
KDISK_DIR = ../kernel-disk
KDISK_FILE= $(KDISK_DIR)/$(BUILD_DIR)/kdisk.img
OVMF_DIR = /usr/share/ovmf/OVMF.fd

#internal project settings
ARCH = x86_64
BOOT_PLATFORM = uefi
BUILD_DIR = build
ASSETS_DIR = assets
TARGET_NAME = kernel-$(ARCH).elf
TARGET = $(BUILD_DIR)/$(TARGET_NAME)

#flags
CXX_INC_DIRS = -I ./include -I $(SYSLIB_DIR)/include -I $(BOOTLOADER_DIR)
CXX_FLAGS = $(ARCH_CXX_FLAGS) -ffreestanding -O0 -fno-rtti -fno-exceptions -mno-red-zone -mcmodel=large -g -Wall -DMINOS_PLATFORM=1
LD_FLAGS = $(ARCH_LD_FLAGS) -ffreestanding -O2 -nostdlib -fno-rtti -fno-exceptions -g -zmax-page-size=0x1000
LD_LIBS = -L $(SYSLIB_DIR)/$(BUILD_DIR) -lsyslib -lgcc
LD_SCRIPT = $(ARCH_DIR)/linker.lds
ASM_FLAGS =
CPP_SRCS = $(shell find -path ./arch -prune -false -o -path ./$(ASSETS_DIR) -prune -false -o -name "*.cpp") $(ARCH_CPP_SRCS)
ASM_SRCS = 
QEMU_FLAGS = -machine q35 -smp cores=4 -serial mon:stdio -m 256M -drive if=pflash,format=raw,readonly,file=$(OVMF_DIR) -cdrom $(TARGET).iso

#architecture settings
ARCH_DIR = arch/$(ARCH)
include $(ARCH_DIR)/make.cfg

#toolchain selection
CXX = x86_64-elf-g++
ASM = x86_64-elf-as
LD = x86_64-elf-gcc

#auto populated vars
CPP_OBJS = $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(CPP_SRCS))
ASM_OBJS = $(patsubst %.asm, $(BUILD_DIR)/%_asm.o, $(ASM_SRCS))
CRTBEGIN_OBJ = $(shell $(CXX) $(CXX_FLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ = $(shell $(CXX) $(CXX_FLAGS) -print-file-name=crtend.o)
CRT_OBJS = $(CRTBEGIN_OBJ) $(CRTEND_OBJ) $(BUILD_DIR)/crti.o $(BUILD_DIR)/crtn.o
BOOT_PLATFORM_DIR = $(BOOTLOADER_DIR)/$(ARCH)-$(BOOT_PLATFORM)
BOOTLOADER_FILE = $(BOOT_PLATFORM_DIR)/build/booty.efi

.PHONY: all clean iso run debug $(TARGET)

all: $(TARGET)

clean:
	@echo DELETING $(BUILD_DIR) ...
	@rm -r $(BUILD_DIR)
	@cd $(BOOT_PLATFORM_DIR); make clean;
	@cd $(SYSLIB_DIR); make clean;
	@cd $(KDISK_DIR); make clean;

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

$(TARGET): $(CPP_OBJS) $(ASM_OBJS) $(LD_SCRIPT) $(CRT_OBJS)
	@cd $(SYSLIB_DIR); make all;
	@cd $(KDISK_DIR); make all;
	@echo LINKING $@
	@$(LD) $(LD_FLAGS) -T $(LD_SCRIPT) $(BUILD_DIR)/crti.o $(CRTBEGIN_OBJ) $(CPP_OBJS) $(ASM_OBJS) $(CRTEND_OBJ) $(BUILD_DIR)/crtn.o -o $@ $(LD_LIBS) -Wl,--format -Wl,binary -Wl,$(KDISK_FILE)

include overrides.mk

$(BUILD_DIR)/%_asm.o: %.asm
	@echo ASSEMBLING $<
	@mkdir -p $(shell dirname $@)
	@$(ASM) $(ASM_FLAGS) $< --64 -o $@

$(BUILD_DIR)/%.o: %.cpp
	@echo COMPILING $<
	@mkdir -p $(shell dirname $@)
	@$(CXX) $(CXX_INC_DIRS) $(CXX_FLAGS) -c $< -o $@
