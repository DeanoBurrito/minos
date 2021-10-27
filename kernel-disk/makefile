INCLUDED_FILES = zap-light16.psf icons/minos-105px.png
TARGET_NAME = kdisk

TARGET = $(BUILD_DIR)/$(TARGET_NAME).bin

.PHONY: all clean

all: $(INCLUDED_FILES)
	@echo Creating kernel init disk ...
	@mkdir -p $(BUILD_DIR)
	@tar -vc $(INCLUDED_FILES) -f $(TARGET)

clean:
	@echo DELETING $(BUILD_DIR) ...
	@rm -r $(BUILD_DIR)
