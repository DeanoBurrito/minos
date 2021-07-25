#external project references
EXT_INCLUDE_DIRS = -I ../bootloader
BOOTLOADER = ../bootloader/build/booty.efi
OVMF_DIR = /usr/share/ovmf/OVMF.fd

#internal project settings
ARCH = x86_64
BUILD_DIR = build
ASSETS_DIR = assets
TARGET_NAME = kernel-$(ARCH).elf
TARGET = $(BUILD_DIR)/$(TARGET_NAME)

#architecture settings
ARCH_DIR = arch/$(ARCH)
include $(ARCH_DIR)/make.cfg

#flags
CXX_INC_DIRS = $(EXT_INCLUDE_DIRS) -I ./include
CXX_FLAGS = $(ARCH_CXX_FLAGS) -ffreestanding -O2 -fno-rtti -fno-exceptions -mno-red-zone -mcmodel=large -ggdb -Wall
LD_FLAGS = $(ARCH_LD_FLAGS) -ffreestanding -O2 -nostdlib -lgcc -g -zmax-page-size=0x1000
LD_SCRIPT = $(ARCH_DIR)/linker.lds
CPP_SRCS = $(shell find -path ./arch -prune -false -o -path ./$(ASSETS_DIR) -prune -false -o -name "*.cpp") $(ARCH_CPP_SRCS)

#toolchain selection
CXX = x86_64-elf-g++
LD = x86_64-elf-gcc

#auto populated vars
CPP_OBJS = $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(CPP_SRCS))

.PHONY: all clean iso run fun

all: $(TARGET)

clean:
	@echo DELETING $(BUILD_DIR) ...
	@rm -r $(BUILD_DIR)
	@cd ../bootloader; make clean;

fun: run
	@echo fun fun fun!

run: iso
	@qemu-system-x86_64 -serial mon:stdio -m 256M -drive if=pflash,format=raw,readonly,file=$(OVMF_DIR) -cdrom $(TARGET).iso

iso: iso-img
	@echo CREATING ISO
	@mkdir -p $(BUILD_DIR)/iso
	@cp $(TARGET).img $(BUILD_DIR)/iso
	@cd $(BUILD_DIR); xorriso -as mkisofs -R -f -e $(TARGET_NAME).img -no-emul-boot -o $(TARGET_NAME).iso iso
	
iso-img: all
	@echo Getting bootloader file
	@cd ../bootloader; make all;
	@cp $(BOOTLOADER) $(BUILD_DIR)/BOOTX64.EFI
	@dd if=/dev/zero of=$(TARGET).img bs=1K count=1440
	@mformat -i $(TARGET).img -f 1440 ::
	@mmd -i $(TARGET).img ::/EFI
	@mmd -i $(TARGET).img ::/EFI/BOOT
	@mcopy -i $(TARGET).img $(BUILD_DIR)/BOOTX64.EFI ::/EFI/BOOT
	@mcopy -i $(TARGET).img $(TARGET) ::
	@mcopy -i $(TARGET).img $(ASSETS_DIR) ::/assets
	@rm $(BUILD_DIR)/BOOTX64.EFI

$(TARGET): $(CPP_OBJS) $(LD_SCRIPT)
	@echo LINKING $@
	@$(LD) $(LD_FLAGS) -T $(LD_SCRIPT) $(CPP_OBJS) -o $@

$(BUILD_DIR)/./Interrupts.o: Interrupts.cpp
	@echo COMPILING $< with special flags
	@mkdir -p $(shell dirname $@)
	@$(CXX) $(CXX_INC_DIRS) $(CPP_FLAGS) -mno-sse -mgeneral-regs-only -c $< -o $@

$(BUILD_DIR)/%.o: %.cpp
	@echo COMPILING $<
	@mkdir -p $(shell dirname $@)
	@$(CXX) $(CXX_INC_DIRS) $(CPP_FLAGS) -c $< -o $@
