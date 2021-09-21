#external project references
BOOTLOADER_DIR = ../boot
SYSLIB_DIR = ../syslib
KDISK_DIR = ../kernel-disk
KDISK_FILE= $(KDISK_DIR)/$(BUILD_DIR)/kdisk.img

#internal project settings
ARCH = x86_64
BOOT_PLATFORM = uefi
BUILD_DIR = build
ASSETS_DIR = assets
TARGET_NAME = kernel-$(ARCH).elf
TARGET = $(BUILD_DIR)/$(TARGET_NAME)

#flags
CXX_INC_DIRS = -I ./include -I $(SYSLIB_DIR)/include -I $(BOOTLOADER_DIR)
CXX_FLAGS = $(ARCH_CXX_FLAGS) -fno-omit-frame-pointer -ffreestanding -O0 -fno-rtti -fno-exceptions -mno-red-zone -mcmodel=large -g -Wall -DMINOS_PLATFORM=1
LD_FLAGS = $(ARCH_LD_FLAGS) -ffreestanding -O2 -nostdlib -fno-rtti -fno-exceptions -g -zmax-page-size=0x1000
LD_LIBS = -L $(SYSLIB_DIR)/$(BUILD_DIR) -lsyslib -lgcc
LD_SCRIPT = $(ARCH_DIR)/linker.lds
ASM_FLAGS =
CPP_SRCS = $(shell find -path ./arch -prune -false -o -path ./$(ASSETS_DIR) -prune -false -o -name "*.cpp" | cut -c3-) $(ARCH_CPP_SRCS)
ASM_SRCS = 

#architecture settings
ARCH_DIR = arch/$(ARCH)
include $(ARCH_DIR)/local.mk
include extras.mk

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
