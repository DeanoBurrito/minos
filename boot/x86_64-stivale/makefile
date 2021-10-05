#external project references
LIMINE_DIR = ../../../../limine

#internal project settings
TARGET_NAME = booty.elf
BUILD_DIR = build
TARGET = $(BUILD_DIR)/$(TARGET_NAME)
CPP_SRCS = Main.cpp
AS_SRCS = Stivale2Header.asm
ISO_DIR = $(BUILD_DIR)/iso
LIMINE_CFG = limine.cfg

#flags
CXX_INC_DIRS = 
CXX_FLAGS = -fPIC -ffreestanding -mno-red-zone -Wall -nostdlib -fno-rtti -fno-exceptions
LD_SCRIPT = linker.lds
LD_FLAGS = -T $(LD_SCRIPT) -ffreestanding -nostdlib

#toolchain selection
CXX = x86_64-elf-g++
LD = x86_64-elf-gcc
AS = x86_64-elf-as

#auto populated vars
OBJS = $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(CPP_SRCS)) $(patsubst %.asm, $(BUILD_DIR)/%_asm.o, $(AS_SRCS))

.PHONY: all clean pack

all: $(OBJS)
	@echo LINKING $(TARGET)
	@$(LD) $(LD_FLAGS) $(OBJS) -o $(TARGET)

pack: all
	@echo Packing kernel with x86_64-limine bootloader
	@mkdir -p $(BUILD_DIR)/iso
	@cp $(LIMINE_CFG) $(BUILD_DIR)/iso
	@cp $(LIMINE_DIR)/limine-cd.bin $(BUILD_DIR)/iso
	@cp $(LIMINE_DIR)/limine.sys $(BUILD_DIR)/iso
	@cp $(LIMINE_DIR)/limine-eltorito-efi.bin $(BUILD_DIR)/iso
	@cp $(BUILD_DIR)/kernel.elf $(BUILD_DIR)/iso
	@cp $(TARGET) $(BUILD_DIR)/iso
	@xorriso -as mkisofs -b limine-cd.bin -no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot limine-eltorito-efi.bin -efi-boot-part --efi-boot-image --protective-msdos-label \
		$(ISO_DIR) -o $(BUILD_DIR)/packed.iso
	@limine-install $(BUILD_DIR)/packed.iso

clean:
	@echo DELETEING build directory
	@rm -rf $(BUILD_DIR)

$(BUILD_DIR)/%_asm.o: %.asm
	@echo ASSEMBLING $<
	@mkdir -p $(BUILD_DIR)
	@$(AS) --64 $< -o $@

$(BUILD_DIR)/%.o: %.cpp
	@echo COMPILING $<
	@mkdir -p $(BUILD_DIR)
	@$(CXX) $(CXX_INC_DIRS) $(CXX_FLAGS) -c $< -o $@
