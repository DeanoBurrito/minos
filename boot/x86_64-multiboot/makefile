TARGET_NAME = multibooty.bin
BUILD_DIR = build
TARGET = $(BUILD_DIR)/$(TARGET_NAME)
CPP_FLAGS = -ffreestanding -fno-rtti -fno-exceptions -Wall -O2 -zmax-page-size=0x1000
LD_FLAGS = -nostdlib -lgcc -T linker.lds

ASM = x86_64-elf-as
CPP = x86_64-elf-gcc

.PHONY: all clean mb-pack

all:
	@echo Building multiboot stub ...
	@mkdir -p $(BUILD_DIR)
	@$(ASM) --64 Main.asm -o $(BUILD_DIR)/Main.asm.o
	@$(CPP) $(CPP_FLAGS) -c Main32.cpp -o $(BUILD_DIR)/Main32.o
	@$(CPP) $(CPP_FLAGS) $(LD_FLAGS) $(BUILD_DIR)/Main.asm.o $(BUILD_DIR)/Main32.o -o $(TARGET)

clean:
	@echo Deleting $(BUILD_DIR) ...
	@rm -r $(BUILD_DIR)

mb-pack: all
	@echo Creating bootable multiboot file ...