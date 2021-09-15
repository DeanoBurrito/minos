#specific files that need to override the default build rules

$(BUILD_DIR)/crti.o: $(ARCH_DIR)/crti.S
	@echo ASSEMBLING $<
	@$(ASM) $(ASM_FLAGS) $< --64 -o $@

$(BUILD_DIR)/crtn.o: $(ARCH_DIR)/crtn.S
	@echo ASSEMBLING $<
	@$(ASM) $(ASM_FLAGS) $< --64 -o $@

$(BUILD_DIR)/arch/x86_64/Interrupts.o: arch/x86_64/Interrupts.cpp
	@echo COMPILING $< with special flags
	@mkdir -p $(shell dirname $@)
	@$(CXX) $(CXX_INC_DIRS) $(CXX_FLAGS) -mno-sse -mgeneral-regs-only -c $< -o $@

$(BUILD_DIR)/multiprocessing/Scheduler.o: multiprocessing/Scheduler.cpp
	@echo COMPILING $< with special flags
	@mkdir -p $(shell dirname $@)
	@$(CXX) $(CXX_INC_DIRS) $(CXX_FLAGS) -mno-sse -mgeneral-regs-only -c $< -o $@

$(BUILD_DIR)/drivers/Ps2Keyboard.o: drivers/Ps2Keyboard.cpp
	@echo COMPILING $< with special flags
	@mkdir -p $(shell dirname $@)
	@$(CXX) $(CXX_INC_DIRS) $(CXX_FLAGS) -mno-sse -mgeneral-regs-only -c $< -o $@