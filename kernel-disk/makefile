BUILD_DIR = build
INCLUDED_FILES = zap-light16.psf icons/minos-105px.png
TARGET_NAME = kdisk

TARGET = $(BUILD_DIR)/$(TARGET_NAME).bin

.PHONY: all clean

all: $(INCLUDED_FILES)
	@mkdir -p $(BUILD_DIR)
	@tar -c $(INCLUDED_FILES) -f $(TARGET)

clean:
	@echo DELETING $(BUILD_DIR) ...
	@rm -r $(BUILD_DIR)
