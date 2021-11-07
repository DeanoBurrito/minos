#generic makefile rules for c++ and asm targets

$(BUILD_DIR)/%.cpp.o: %.cpp
	@echo Compling cpp file $<
	@mkdir -p $(shell dirname $@)
	@$(CXX) $(CXX_FLAGS) -c $< -o $@

$(BUILD_DIR)/%.asm.o: %.asm
	@echo Assembling file $<
	@mkdir -p $(shell dirname $@)
	@$(ASM) $(ASM_FLAGS) $< --64 -o $@
