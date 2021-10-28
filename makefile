#toolchain location
TOOLCHAIN_DIR = ../minos-tools
CXX_BIN = $(CPU_ARCH)-elf-g++
ASM_BIN = $(CPU_ARCH)-elf-as
LD_BIN = $(CPU_ARCH)-elf-gcc
export TOOLS_DIR = $(abspath $(TOOLCHAIN_DIR))
export CXX = $(abspath $(TOOLCHAIN_DIR)/bin/$(CXX_BIN))
export ASM = $(abspath $(TOOLCHAIN_DIR)/bin/$(ASM_BIN))
export LD = $(abspath $(TOOLCHAIN_DIR)/bin/$(LD_BIN))

#various projects (syslib/userlib need to be linked against, so we export them)
PROJ_BOOTLOADER_DIR = boot
PROJ_SYSLIB_DIR = syslib
PROJ_KDISK_DIR = kernel-disk
PROJ_KERNEL_DIR = kernel
PROJ_USERLIB_DIR = userlib
PROJ_APPS_DIR = apps
export SYSLIB_WHERE = $(abspath $(PROJ_SYSLIB_DIR))
export USERLIB_WHERE = $(abspath $(PROJ_USERLIB_DIR))
export KERNEL_FILENAME = kernel-$(ARCH).elf
export KERNEL_FULL_FILEPATH = $(abspath $(PROJ_KERNEL_DIR)/$(BUILD_DIR)/$(KERNEL_FILENAME))

#build configuration
export CPU_ARCH = x86_64
export BOOT_ARCH = uefi
export INCLUDE_DEBUG_INFO=1
export OPTIMIZATION_LEVEL=2
export BUILD_DIR=build

ISO_FILENAME = iso/minos-$(BOOTLOADER_ARCH).iso
export ISO_TARGET=$(abspath $(ISO_FILENAME))

#magic internal stuff
BOOTLOADER_ARCH = $(CPU_ARCH)-$(BOOT_ARCH)
BOOTLOADER_ARCH_DIR = $(PROJ_BOOTLOADER_DIR)/$(BOOTLOADER_ARCH)

ifeq ($(INCLUDE_DEBUG_INFO), 1)
	export CXX_DEBUG_FLAGS = -g
else
	export CXX_DEBUG_FLAGS =
endif

#bring in run and debug commands
include run.mk

.PHONY: top-rule clean clean-apps validate-toolchain all no-apps core run debug

#so that running 'make' with no args will default to building everything
top-rule: all

clean:
	@echo "Cleaning all directories (use clean-apps to clean app dirs) ..."
	@-cd $(BOOTLOADER_ARCH_DIR); make clean
	@-cd $(PROJ_KDISK_DIR); make clean
	@-cd $(PROJ_SYSLIB_DIR); make clean
	@-cd $(PROJ_KERNEL_DIR); make clean
	@-cd $(PROJ_USERLIB_DIR); make clean
	@-rm -rf iso

clean-apps:
	@echo "Cleaning app build directories ..."
	@-cd $(PROJ_APPS_DIR); make clean

validate-toolchain:
	@echo "Checking for C++ compiler"
	@which $(CXX) || echo "Could not locate architecture specific C++ compiler."
	@echo "Checking for assembler"
	@which $(ASM) || echo "Could not locate architecture specific assembler."
	@echo "Checking for linker"
	@which $(LD) || echo "Could not locate architecture specific linker."
	@echo "Checking for gnu-efi install"
	@if [ -d "$(TOOLS_DIR)/gnu-efi" ] ; then echo "Found gnu-efi."; else echo "Could not find gnu-efi files."; fi
	@echo "Checking for limine install"
	@if [ -d "$(TOOLS_DIR)/limine" ] ; then echo "Found limine."; else echo "Unable to locate limine."; fi

prep-build-env:
	@mkdir -p $(shell dirname $(ISO_TARGET))

all: prep-build-env
	@echo "Starting minos build (all) ..."
	@cd $(BOOTLOADER_ARCH_DIR); make all;
	@cd $(PROJ_KDISK_DIR); make all;
	@cd $(PROJ_SYSLIB_DIR); make all;
	@cd $(PROJ_KERNEL_DIR); make all -e KDISK_FILE="../$(PROJ_KDISK_DIR)/build/kdisk.bin";
	@cd $(PROJ_USERLIB_DIR); make all;
	@cd $(PROJ_APPS_DIR); make all;
	@cd $(BOOTLOADER_ARCH_DIR); make pack -e BOOT_PACK_FILES=$(abspath LICENSE) -e BOOT_KERNEL_FILE=$(KERNEL_FULL_FILEPATH);

no-apps: prep-build-env
	@echo "Starting minos build (userland) ..."
	@cd $(BOOTLOADER_ARCH_DIR); make all;
	@cd $(PROJ_KDISK_DIR); make all;
	@cd $(PROJ_SYSLIB_DIR); make all;
	@cd $(PROJ_KERNEL_DIR); make all -e KDISK_FILE="../$(PROJ_KDISK_DIR)/build/kdisk.bin";
	@cd $(PROJ_USERLIB_DIR); make all;
	@cd $(BOOTLOADER_ARCH_DIR); make pack -e BOOT_PACK_FILES=$(abspath LICENSE) -e BOOT_KERNEL_FILE=$(KERNEL_FULL_FILEPATH);

core: prep-build-env
	@echo "Starting minos build (core) ..."
	@cd $(BOOTLOADER_ARCH_DIR); make all;
	@cd $(PROJ_KDISK_DIR); make all;
	@cd $(PROJ_SYSLIB_DIR); make all;
	@cd $(PROJ_KERNEL_DIR); make all -e KDISK_FILE="../$(PROJ_KDISK_DIR)/build/kdisk.bin";
	@cd $(BOOTLOADER_ARCH_DIR); make pack -e BOOT_PACK_FILES=$(abspath LICENSE) -e BOOT_KERNEL_FILE=$(KERNEL_FULL_FILEPATH);
