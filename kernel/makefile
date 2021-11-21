#global settings - this will be overriden by top level makefile
KDISK_FILE =
KERNEL_FILENAME = 

#internal project settings
TARGET = $(BUILD_DIR)/$(KERNEL_FILENAME)

#flags
CXX_INC_DIRS = -Iinclude -I$(SYSLIB_WHERE)/include -I../boot
CXX_FLAGS = $(ARCH_CXX_FLAGS) -fno-stack-protector -fno-omit-frame-pointer -ffreestanding -O$(OPTIMIZATION_LEVEL) -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-rtti -fno-exceptions -mno-red-zone -mcmodel=large -Wall -DMINOS_PLATFORM=1 -std=c++17 $(CXX_DEBUG_FLAGS) -fpie
LD_FLAGS = $(ARCH_LD_FLAGS) -nostdlib -zmax-page-size=0x1000 $(CXX_DEBUG_FLAGS) -static -pie --no-dynamic-linker -ztext
LD_LIBS = -L$(SYSLIB_WHERE)/build -L$(TOOLS_DIR)/lib/gcc/x86_64-elf/11.2.0 -lsyslib -lgcc
LD_SCRIPT = $(ARCH_DIR)/linker.lds
ASM_FLAGS =
CPP_SRCS = $(shell find -path ./arch -prune -false -o -name "*.cpp" | cut -c3-) $(ARCH_CPP_SRCS)
ASM_SRCS = 

#architecture settings
ARCH_DIR = arch/$(CPU_ARCH)
include $(ARCH_DIR)/local.mk

#auto populated vars
CPP_OBJS = $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(CPP_SRCS))
ASM_OBJS = $(patsubst %.asm, $(BUILD_DIR)/%_asm.o, $(ASM_SRCS))
CRTBEGIN_OBJ = $(shell $(CXX) $(CXX_FLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ = $(shell $(CXX) $(CXX_FLAGS) -print-file-name=crtend.o)
CRT_OBJS = $(CRTBEGIN_OBJ) $(CRTEND_OBJ) $(BUILD_DIR)/crti.o $(BUILD_DIR)/crtn.o

.PHONY: all clean iso $(TARGET)

all: $(TARGET)

clean:
	@echo DELETING $(BUILD_DIR) ...
	@-rm -r $(BUILD_DIR)

$(TARGET): $(CPP_OBJS) $(ASM_OBJS) $(LD_SCRIPT) $(CRT_OBJS)
	@echo LINKING $@
	@$(LD) -T $(LD_SCRIPT) $(BUILD_DIR)/crti.o $(CRTBEGIN_OBJ) $(CPP_OBJS) $(ASM_OBJS) $(CRTEND_OBJ) $(BUILD_DIR)/crtn.o -o $@ $(LD_FLAGS) $(LD_LIBS) --format binary $(KDISK_FILE)

include overrides.mk

$(BUILD_DIR)/%_asm.o: %.asm
	@echo ASSEMBLING $<
	@mkdir -p $(shell dirname $@)
	@$(ASM) $(ASM_FLAGS) $< --64 -o $@

$(BUILD_DIR)/%.o: %.cpp
	@echo COMPILING $<
	@mkdir -p $(shell dirname $@)
	@$(CXX) $(CXX_INC_DIRS) $(CXX_FLAGS) -c $< -o $@
