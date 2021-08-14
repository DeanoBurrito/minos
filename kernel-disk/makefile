BUILD_DIR = build
INCLUDED_FILES = example.txt
TARGET_NAME = kdisk

TARGET = $(BUILD_DIR)/$(TARGET_NAME).img

.PHONY: all clean

all: $(INCLUDED_FILES)
	@mkdir -p $(BUILD_DIR)
	@dd if=/dev/zero of=$(TARGET) bs=1K count=1440
	@mformat -i $(TARGET) -f 1440 ::
	@mmd -i $(TARGET) ::/$(TARGET_NAME)
	@mcopy -i $(TARGET) -s $(INCLUDED_FILES) ::/$(TARGET_NAME)

clean:
	@echo DELETING $(BUILD_DIR) ...
	@rm -r $(BUILD_DIR)
