TARGET_NAME = multibooty.bin
BUILD_DIR = build
TARGET = $(BUILD_DIR)/$(TARGET_NAME)
CPP_FLAGS = -ffreestanding -fno-rtti -fno-exceptions -Wall -O2 -zmax-page-size=0x1000 -g
LD_FLAGS = -nostdlib -lgcc -T linker.lds

ASM = i686-elf-as
CPP = i686-elf-gcc

.PHONY: all clean mb-pack

all:
	@echo Building multiboot stub ...
	@mkdir -p $(BUILD_DIR)
	@$(ASM) --32 Main.asm -o $(BUILD_DIR)/Main.asm.o
	@$(CPP) $(CPP_FLAGS) -c Main32.cpp -o $(BUILD_DIR)/Main32.o
	@$(CPP) $(CPP_FLAGS) -c Loader.cpp -o $(BUILD_DIR)/Loader.o
	@$(CPP) $(CPP_FLAGS) $(LD_FLAGS) $(BUILD_DIR)/Main.asm.o $(BUILD_DIR)/Main32.o $(BUILD_DIR)/Loader.o -o $(TARGET)

clean:
	@echo Deleting $(BUILD_DIR) ...
	@rm -r $(BUILD_DIR)

mb-pack: all
	@echo Creating bootable multiboot file ...
	@tar -c $(BUILD_DIR)/kernel.elf -f $(BUILD_DIR)/kernel.tar
	@objcopy --input binary --output elf32-i386 $(BUILD_DIR)/kernel.tar $(BUILD_DIR)/kernel.o
	@$(CPP) $(CPP_FLAGS) $(LD_FLAGS) $(BUILD_DIR)/Main.asm.o $(BUILD_DIR)/Main32.o $(BUILD_DIR)/Loader.o $(BUILD_DIR)/kernel.o -o $(BUILD_DIR)/kernel-patched.elf

run: all
	@echo Running QEMU
	@qemu-system-x86_64 -kernel $(TARGET) -serial mon:stdio

debug: all
	@echo Debugging in QEMU
	@qemu-system-x86_64 -kernel $(TARGET) -serial mon:stdio -s -S