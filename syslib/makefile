#internal settings
BUILD_DIR = build
TARGET_NAME = libsyslib.a
TARGET = $(BUILD_DIR)/$(TARGET_NAME)

#flags
CXX_INC_DIRS = -I ./include
CXX_FLAGS = -fno-omit-frame-pointer -ffreestanding -O2 -fno-rtti -fno-exceptions -mno-red-zone -mcmodel=large -Wall
CPP_SRCS = $(shell find -name "*.cpp")

#toolchain selection
CXX = x86_64-elf-g++
AR = x86_64-elf-ar

#auto populated vars
CPP_OBJS = $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(CPP_SRCS))

.PHONY: all clean

all: $(TARGET)

clean:
	@echo DELETING $(BUILD_DIR) ...
	@rm -r $(BUILD_DIR)

$(TARGET): $(CPP_OBJS)
	@echo ARCHIVING static lib ...
	@$(AR) -rcs $(TARGET) $(CPP_OBJS)

$(BUILD_DIR)/%.o: %.cpp
	@echo COMPILING $<
	@mkdir -p $(shell dirname $@)
	@$(CXX) $(CXX_INC_DIRS) $(CXX_FLAGS) -c $< -o $@
