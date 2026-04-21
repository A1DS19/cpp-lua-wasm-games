# Project variables
PROJECT_NAME := $(shell basename $(CURDIR))
BUILD_DIR := build
BIN_DIR := bin
SRC_DIR := src
INCLUDE_DIR := include
TESTS_DIR := tests
SCRIPTS_DIR := scripts
LIB_DIR := lib
SIM_DIR := sim

# Detect platform
ifeq ($(OS),Windows_NT)
    PLATFORM := Windows
    EXECUTABLE_EXT := .exe
    CMAKE_GENERATOR := "Visual Studio 17 2022"
    RM := del /Q /S
    RMDIR := rmdir /S /Q
    MKDIR := mkdir
    SHELL_CHECK := where
    NULL_DEVICE := nul
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        PLATFORM := Linux
        EXECUTABLE_EXT :=
        CMAKE_GENERATOR := "Unix Makefiles"
    endif
    ifeq ($(UNAME_S),Darwin)
        PLATFORM := macOS
        EXECUTABLE_EXT :=
        CMAKE_GENERATOR := "Unix Makefiles"
    endif
    RM := rm -rf
    RMDIR := rm -rf
    MKDIR := mkdir -p
    SHELL_CHECK := command -v
    NULL_DEVICE := /dev/null
endif

# Build configurations
BUILD_TYPE ?= Debug
CXXFLAGS ?=
CMAKE_FLAGS := -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Help CMake find Homebrew packages on macOS (Apple Silicon: /opt/homebrew, Intel: /usr/local)
ifeq ($(PLATFORM),macOS)
    HOMEBREW_PREFIX := $(shell brew --prefix 2>/dev/null || echo /opt/homebrew)
    CMAKE_FLAGS += -DCMAKE_PREFIX_PATH=$(HOMEBREW_PREFIX)
endif
ifneq ($(CXXFLAGS),)
    CMAKE_FLAGS += -DCMAKE_CXX_FLAGS="$(CXXFLAGS)"
endif

# Tool detection
HAS_CMAKE      := $(shell $(SHELL_CHECK) cmake      >$(NULL_DEVICE) 2>&1 && echo "yes" || echo "no")
HAS_GXX        := $(shell $(SHELL_CHECK) g++        >$(NULL_DEVICE) 2>&1 && echo "yes" || echo "no")
HAS_CLANGXX    := $(shell $(SHELL_CHECK) clang++    >$(NULL_DEVICE) 2>&1 && echo "yes" || echo "no")
HAS_CLANG_FORMAT := $(shell $(SHELL_CHECK) clang-format >$(NULL_DEVICE) 2>&1 && echo "yes" || echo "no")
HAS_CPPCHECK   := $(shell $(SHELL_CHECK) cppcheck   >$(NULL_DEVICE) 2>&1 && echo "yes" || echo "no")
HAS_VALGRIND   := $(shell $(SHELL_CHECK) valgrind   >$(NULL_DEVICE) 2>&1 && echo "yes" || echo "no")
HAS_GDB        := $(shell $(SHELL_CHECK) gdb        >$(NULL_DEVICE) 2>&1 && echo "yes" || echo "no")
HAS_LLDB       := $(shell $(SHELL_CHECK) lldb       >$(NULL_DEVICE) 2>&1 && echo "yes" || echo "no")
HAS_GCOV       := $(shell $(SHELL_CHECK) gcov       >$(NULL_DEVICE) 2>&1 && echo "yes" || echo "no")
HAS_DOXYGEN    := $(shell $(SHELL_CHECK) doxygen    >$(NULL_DEVICE) 2>&1 && echo "yes" || echo "no")
HAS_IVERILOG   := $(shell $(SHELL_CHECK) iverilog   >$(NULL_DEVICE) 2>&1 && echo "yes" || echo "no")
HAS_VERILATOR  := $(shell $(SHELL_CHECK) verilator  >$(NULL_DEVICE) 2>&1 && echo "yes" || echo "no")
HAS_GTKWAVE    := $(shell $(SHELL_CHECK) gtkwave    >$(NULL_DEVICE) 2>&1 && echo "yes" || echo "no")
HAS_YOSYS      := $(shell $(SHELL_CHECK) yosys      >$(NULL_DEVICE) 2>&1 && echo "yes" || echo "no")

# Verilog configuration
VERILOG_SRC    := $(wildcard $(SIM_DIR)/*.v $(SIM_DIR)/**/*.v)
VERILOG_TOP    ?= top
VVP_OUT        := $(BUILD_DIR)/$(VERILOG_TOP).vvp
VCD_OUT        := $(BUILD_DIR)/$(VERILOG_TOP).vcd

# Colors for output (if terminal supports it)
ifndef NO_COLOR
    RED    := $(shell printf '\033[31m')
    GREEN  := $(shell printf '\033[32m')
    YELLOW := $(shell printf '\033[33m')
    BLUE   := $(shell printf '\033[34m')
    CYAN   := $(shell printf '\033[36m')
    RESET  := $(shell printf '\033[0m')
    BOLD   := $(shell printf '\033[1m')
else
    RED    :=
    GREEN  :=
    YELLOW :=
    BLUE   :=
    CYAN   :=
    RESET  :=
    BOLD   :=
endif

# Default target
.PHONY: all
all: build

# Help target
.PHONY: help
help:
	@echo "$(BOLD)$(BLUE)Available targets for C++ development on $(PLATFORM):$(RESET)"
	@echo ""
	@echo "$(BOLD)$(GREEN)Building:$(RESET)"
	@echo "  $(CYAN)build$(RESET)       - Build the project (default)"
	@echo "  $(CYAN)rebuild$(RESET)     - Clean and build"
	@echo "  $(CYAN)release$(RESET)     - Build optimized release version"
	@echo "  $(CYAN)install$(RESET)     - Install to system (requires sudo on Unix)"
	@echo "  $(CYAN)uninstall$(RESET)   - Remove from system (requires sudo on Unix)"
	@echo ""
	@echo "$(BOLD)$(GREEN)Running & Testing:$(RESET)"
	@echo "  $(CYAN)run$(RESET)         - Build and run the main executable"
	@echo "  $(CYAN)test$(RESET)        - Build and run tests"
	@echo "  $(CYAN)debug$(RESET)       - Build and run with debugger"
	@echo "  $(CYAN)valgrind$(RESET)    - Run with Valgrind (Linux/macOS only)"
	@echo "  $(CYAN)memcheck$(RESET)    - Comprehensive memory analysis"
	@echo ""
	@echo "$(BOLD)$(GREEN)Code Quality:$(RESET)"
	@echo "  $(CYAN)format$(RESET)      - Format code with clang-format"
	@echo "  $(CYAN)analyze$(RESET)     - Run static analysis tools"
	@echo "  $(CYAN)lint$(RESET)        - Run linting checks"
	@echo "  $(CYAN)coverage$(RESET)    - Generate code coverage report"
	@echo "  $(CYAN)setup-lsp$(RESET)   - Setup clangd configuration"
	@echo ""
	@echo "$(BOLD)$(GREEN)Verilog:$(RESET)"
	@echo "  $(CYAN)sim$(RESET)         - Compile and simulate Verilog (iverilog)"
	@echo "  $(CYAN)wave$(RESET)        - Open waveform viewer (gtkwave)"
	@echo "  $(CYAN)synth$(RESET)       - Synthesize with Yosys"
	@echo "  $(CYAN)lint-verilog$(RESET) - Lint Verilog with Verilator"
	@echo ""
	@echo "$(BOLD)$(GREEN)Maintenance:$(RESET)"
	@echo "  $(CYAN)clean$(RESET)       - Clean build artifacts"
	@echo "  $(CYAN)distclean$(RESET)   - Deep clean (including CMake cache)"
	@echo "  $(CYAN)deps$(RESET)        - Show build dependencies"
	@echo "  $(CYAN)check$(RESET)       - Check build environment"
	@echo "  $(CYAN)info$(RESET)        - Show project information"
	@echo ""
	@echo "$(BOLD)$(GREEN)WebAssembly (Emscripten):$(RESET)"
	@echo "  $(CYAN)wasm$(RESET)        - Build for WebAssembly with Emscripten"
	@echo "  $(CYAN)wasm-run$(RESET)    - Build and open in browser (emrun)"
	@echo "  $(CYAN)wasm-clean$(RESET)  - Clean WebAssembly build artifacts"
	@echo ""
	@echo "$(BOLD)$(GREEN)Development:$(RESET)"
	@echo "  $(CYAN)watch$(RESET)       - Watch for changes and auto-build"
	@echo "  $(CYAN)benchmark$(RESET)   - Run performance benchmarks"
	@echo "  $(CYAN)docs$(RESET)        - Generate HTML documentation (requires doxygen)"
	@echo "  $(CYAN)open-docs$(RESET)   - Open documentation in browser"
	@echo "  $(CYAN)help$(RESET)        - Show this help message"

# Build target
.PHONY: build
build: check-cmake $(BUILD_DIR)/Makefile
	@echo "$(BOLD)$(BLUE)Building $(PROJECT_NAME) for $(PLATFORM)...$(RESET)"
	@cmake --build $(BUILD_DIR) --config $(BUILD_TYPE)
	@echo "$(GREEN)Build complete!$(RESET)"
ifeq ($(HAS_DOXYGEN),yes)
	@echo "$(BOLD)$(BLUE)Generating documentation...$(RESET)"
	@doxygen Doxyfile
	@echo "$(GREEN)Docs: docs/html/index.html$(RESET)"
endif

# Configure CMake
$(BUILD_DIR)/Makefile: CMakeLists.txt
	@echo "$(BOLD)$(BLUE)Configuring CMake for $(PLATFORM)...$(RESET)"
	@$(MKDIR) $(BUILD_DIR) 2>$(NULL_DEVICE) || true
	@cmake -B $(BUILD_DIR) -S . -G $(CMAKE_GENERATOR) $(CMAKE_FLAGS) -DGAME=$(GAME)

# Clean target
.PHONY: clean
clean:
	@echo "$(BOLD)$(YELLOW)Cleaning build artifacts...$(RESET)"
ifeq ($(PLATFORM),Windows)
	@if exist $(BUILD_DIR) $(RMDIR) $(BUILD_DIR) 2>$(NULL_DEVICE) || true
	@if exist $(BIN_DIR) $(RMDIR) $(BIN_DIR) 2>$(NULL_DEVICE) || true
	@if exist $(LIB_DIR) $(RMDIR) $(LIB_DIR) 2>$(NULL_DEVICE) || true
	@$(MKDIR) $(BUILD_DIR) $(BIN_DIR) $(LIB_DIR) 2>$(NULL_DEVICE) || true
else
	@$(RM) $(BUILD_DIR)/* $(BIN_DIR)/* $(LIB_DIR)/* compile_commands.json 2>$(NULL_DEVICE) || true
	@$(RM) *.gcno *.gcda *.gcov coverage/ valgrind-report.log analysis-results/ 2>$(NULL_DEVICE) || true
	@touch $(BUILD_DIR)/.gitkeep $(BIN_DIR)/.gitkeep $(LIB_DIR)/.gitkeep
endif
	@echo "$(GREEN)Clean complete!$(RESET)"

# Deep clean target
.PHONY: distclean
distclean:
	@echo "$(BOLD)$(YELLOW)Deep cleaning (including CMake cache)...$(RESET)"
ifeq ($(PLATFORM),Windows)
	@if exist $(BUILD_DIR) $(RMDIR) $(BUILD_DIR) 2>$(NULL_DEVICE) || true
	@if exist $(BIN_DIR) $(RMDIR) $(BIN_DIR) 2>$(NULL_DEVICE) || true
	@if exist $(LIB_DIR) $(RMDIR) $(LIB_DIR) 2>$(NULL_DEVICE) || true
	@$(MKDIR) $(BUILD_DIR) $(BIN_DIR) $(LIB_DIR) 2>$(NULL_DEVICE) || true
else
	@$(RM) $(BUILD_DIR) $(BIN_DIR)/* $(LIB_DIR)/* compile_commands.json 2>$(NULL_DEVICE) || true
	@$(RM) *.gcno *.gcda *.gcov coverage/ valgrind-report.log analysis-results/ 2>$(NULL_DEVICE) || true
	@$(MKDIR) $(BUILD_DIR) $(BIN_DIR) $(LIB_DIR)
	@touch $(BUILD_DIR)/.gitkeep $(BIN_DIR)/.gitkeep $(LIB_DIR)/.gitkeep
endif
	@echo "$(GREEN)Deep clean complete!$(RESET)"

# Rebuild target
.PHONY: rebuild
rebuild: clean build

# Run target
.PHONY: run
run: rebuild
	@echo "$(BOLD)$(BLUE)Running $(PROJECT_NAME)...$(RESET)"
ifeq ($(PLATFORM),Windows)
	@$(BIN_DIR)\main$(EXECUTABLE_EXT)
else
	@LSAN_OPTIONS=suppressions=$(CURDIR)/$(SCRIPTS_DIR)/lsan.supp:print_suppressions=0 \
		./$(BIN_DIR)/main$(EXECUTABLE_EXT)
endif

# Debug target
.PHONY: debug
debug: BUILD_TYPE = Debug
debug: build
	@echo "$(BOLD)$(BLUE)Running $(PROJECT_NAME) with debugger...$(RESET)"
ifeq ($(PLATFORM),Windows)
	@echo "$(YELLOW)Start debugging with your preferred debugger$(RESET)"
	@$(BIN_DIR)\main$(EXECUTABLE_EXT)
else ifeq ($(PLATFORM),macOS)
ifeq ($(HAS_LLDB),yes)
	@lldb ./$(BIN_DIR)/main$(EXECUTABLE_EXT)
else
	@echo "$(RED)lldb not found. Install Xcode command line tools.$(RESET)"
endif
else
ifeq ($(HAS_GDB),yes)
	@gdb -tui ./$(BIN_DIR)/main$(EXECUTABLE_EXT)
else
	@echo "$(RED)gdb not found. Install with: sudo apt install gdb$(RESET)"
endif
endif

# Valgrind target (Linux/macOS only)
.PHONY: valgrind
valgrind: CMAKE_FLAGS += -DENABLE_SANITIZERS=OFF
valgrind: BUILD_TYPE = Debug
valgrind: build
ifeq ($(PLATFORM),Windows)
	@echo "$(YELLOW)Valgrind not available on Windows. Use Application Verifier instead.$(RESET)"
else
ifeq ($(HAS_VALGRIND),yes)
	@echo "$(BOLD)$(BLUE)Running $(PROJECT_NAME) with Valgrind...$(RESET)"
	@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
		--error-exitcode=1 ./$(BIN_DIR)/main$(EXECUTABLE_EXT)
else
	@echo "$(RED)Valgrind not found.$(RESET)"
ifeq ($(PLATFORM),Linux)
	@echo "Install with: sudo apt install valgrind"
else
	@echo "Install with: brew install valgrind"
endif
endif
endif

# Comprehensive memory check
.PHONY: memcheck
memcheck: BUILD_TYPE = Debug
memcheck: build
	@echo "$(BOLD)$(BLUE)Running comprehensive memory check...$(RESET)"
ifeq ($(PLATFORM),Windows)
	@echo "$(YELLOW)Using built-in runtime checks on Windows$(RESET)"
	@$(BIN_DIR)\main$(EXECUTABLE_EXT)
else
	@if [ -f $(SCRIPTS_DIR)/memcheck.sh ]; then \
		bash $(SCRIPTS_DIR)/memcheck.sh; \
	else \
		$(MAKE) valgrind; \
	fi
endif

# Release build
.PHONY: release
release: BUILD_TYPE = Release
release: distclean build
	@echo "$(GREEN)Release build complete!$(RESET)"

# Test target
.PHONY: test
test: BUILD_TYPE = Debug
test: build
	@echo "$(BOLD)$(BLUE)Building and running tests...$(RESET)"
	@cmake --build $(BUILD_DIR) --target test_main --config $(BUILD_TYPE) 2>$(NULL_DEVICE) || echo "$(YELLOW)No tests configured$(RESET)"
ifeq ($(PLATFORM),Windows)
	@if exist $(BIN_DIR)\test_main$(EXECUTABLE_EXT) ( \
		echo "Running tests..." && \
		$(BIN_DIR)\test_main$(EXECUTABLE_EXT) \
	) else ( \
		echo "$(YELLOW)No test executable found$(RESET)" \
	)
else
	@if [ -f $(BIN_DIR)/test_main$(EXECUTABLE_EXT) ]; then \
		echo "Running tests..." && \
		./$(BIN_DIR)/test_main$(EXECUTABLE_EXT); \
	else \
		echo "$(YELLOW)No test executable found$(RESET)"; \
	fi
endif

# Format target
.PHONY: format
format:
	@echo "$(BOLD)$(BLUE)Formatting C++ code...$(RESET)"
ifeq ($(HAS_CLANG_FORMAT),yes)
	@find $(SRC_DIR) $(INCLUDE_DIR) $(TESTS_DIR) -name "*.cpp" -o -name "*.hpp" -o -name "*.h" 2>$(NULL_DEVICE) | \
		xargs clang-format -i
	@echo "$(GREEN)Code formatting complete!$(RESET)"
else
	@echo "$(RED)clang-format not found.$(RESET)"
ifeq ($(PLATFORM),Linux)
	@echo "Install with: sudo apt install clang-format"
else ifeq ($(PLATFORM),macOS)
	@echo "Install with: brew install clang-format"
else
	@echo "Install via MSYS2: pacman -S mingw-w64-x86_64-clang"
endif
endif

# Static analysis target
.PHONY: analyze
analyze: build
	@echo "$(BOLD)$(BLUE)Running static analysis...$(RESET)"
	@if [ -f $(SCRIPTS_DIR)/analyze.sh ]; then \
		bash $(SCRIPTS_DIR)/analyze.sh; \
	else \
		echo "$(YELLOW)Analysis script not found$(RESET)"; \
	fi

# Lint target
.PHONY: lint
lint: analyze

# Coverage target
.PHONY: coverage
coverage: BUILD_TYPE = Debug
coverage:
	@echo "$(BOLD)$(BLUE)Generating code coverage...$(RESET)"
	@if [ -f $(SCRIPTS_DIR)/coverage.sh ]; then \
		bash $(SCRIPTS_DIR)/coverage.sh; \
	else \
		echo "$(YELLOW)Coverage script not found$(RESET)"; \
	fi

# Setup LSP target
.PHONY: setup-lsp
setup-lsp:
	@echo "$(BOLD)$(BLUE)Setting up LSP configuration...$(RESET)"
	@bash scripts/setup_clangd.sh

# Install target
.PHONY: install
install: release
	@echo "$(BOLD)$(BLUE)Installing $(PROJECT_NAME)...$(RESET)"
ifeq ($(PLATFORM),Windows)
	@echo "$(YELLOW)Manual installation required on Windows$(RESET)"
	@echo "Copy $(BIN_DIR)\\main$(EXECUTABLE_EXT) to your desired location"
else
	@sudo cp $(BIN_DIR)/main$(EXECUTABLE_EXT) /usr/local/bin/$(PROJECT_NAME)
	@sudo chmod +x /usr/local/bin/$(PROJECT_NAME)
	@echo "$(GREEN)Installed to /usr/local/bin/$(PROJECT_NAME)$(RESET)"
endif

# Uninstall target
.PHONY: uninstall
uninstall:
	@echo "$(BOLD)$(YELLOW)Uninstalling $(PROJECT_NAME)...$(RESET)"
ifeq ($(PLATFORM),Windows)
	@echo "$(YELLOW)Manual uninstallation required on Windows$(RESET)"
else
	@sudo rm -f /usr/local/bin/$(PROJECT_NAME)
	@echo "$(GREEN)Uninstalled from /usr/local/bin/$(PROJECT_NAME)$(RESET)"
endif

# Show dependencies
.PHONY: deps
deps:
	@echo "$(BOLD)$(BLUE)Build dependencies for C++ development on $(PLATFORM):$(RESET)"
	@echo ""
	@echo "$(BOLD)$(GREEN)Required:$(RESET)"
	@echo "  - CMake 3.14+"
	@echo "  - C++23 compatible compiler (GCC 13+, Clang 16+, MSVC 2022+)"
	@echo "  - make"
	@echo ""
	@echo "$(BOLD)$(GREEN)Optional:$(RESET)"
	@echo "  - clang-format (code formatting)"
	@echo "  - cppcheck (static analysis)"
	@echo "  - clang-tidy (enhanced static analysis)"
	@echo "  - scan-build (Clang static analyzer)"
ifeq ($(PLATFORM),Windows)
	@echo "  - Application Verifier (memory checking)"
else
	@echo "  - valgrind (memory checking)"
	@echo "  - gdb/lldb (debugging)"
endif
	@echo "  - lcov/gcov (code coverage)"
	@echo ""
	@echo "$(BOLD)$(GREEN)Verilog (optional):$(RESET)"
	@echo "  - iverilog (simulation)"
	@echo "  - verilator (lint + fast simulation)"
	@echo "  - gtkwave (waveform viewer)"
	@echo "  - yosys (synthesis)"

# Check build environment
.PHONY: check
check:
	@echo "$(BOLD)$(BLUE)Checking C++ development environment for $(PLATFORM)...$(RESET)"
	@echo ""
ifeq ($(HAS_CMAKE),yes)
	@echo "$(GREEN)CMake found$(RESET)"
else
	@echo "$(RED)CMake not found$(RESET)"
endif
	@$(SHELL_CHECK) make >$(NULL_DEVICE) 2>&1 && echo "$(GREEN)Make found$(RESET)" || echo "$(RED)Make not found$(RESET)"
ifeq ($(HAS_GXX),yes)
	@echo "$(GREEN)g++ found$(RESET)"
else ifeq ($(HAS_CLANGXX),yes)
	@echo "$(GREEN)clang++ found$(RESET)"
else
	@echo "$(RED)No C++ compiler found$(RESET)"
endif
ifeq ($(HAS_CLANG_FORMAT),yes)
	@echo "$(GREEN)clang-format found$(RESET)"
else
	@echo "$(YELLOW)clang-format not found (optional)$(RESET)"
endif
ifeq ($(HAS_CPPCHECK),yes)
	@echo "$(GREEN)cppcheck found$(RESET)"
else
	@echo "$(YELLOW)cppcheck not found (optional)$(RESET)"
endif
ifeq ($(HAS_GCOV),yes)
	@echo "$(GREEN)gcov found$(RESET)"
else
	@echo "$(YELLOW)gcov not found (optional)$(RESET)"
endif
ifneq ($(PLATFORM),Windows)
ifeq ($(HAS_VALGRIND),yes)
	@echo "$(GREEN)valgrind found$(RESET)"
else
	@echo "$(YELLOW)valgrind not found (optional)$(RESET)"
endif
ifeq ($(HAS_GDB),yes)
	@echo "$(GREEN)gdb found$(RESET)"
else ifeq ($(HAS_LLDB),yes)
	@echo "$(GREEN)lldb found$(RESET)"
else
	@echo "$(YELLOW)no debugger found (optional)$(RESET)"
endif
endif
	@echo ""
	@echo "$(BOLD)$(BLUE)Verilog tools:$(RESET)"
ifeq ($(HAS_IVERILOG),yes)
	@echo "$(GREEN)iverilog found$(RESET)"
else
	@echo "$(YELLOW)iverilog not found (optional)$(RESET)"
endif
ifeq ($(HAS_VERILATOR),yes)
	@echo "$(GREEN)verilator found$(RESET)"
else
	@echo "$(YELLOW)verilator not found (optional)$(RESET)"
endif
ifeq ($(HAS_GTKWAVE),yes)
	@echo "$(GREEN)gtkwave found$(RESET)"
else
	@echo "$(YELLOW)gtkwave not found (optional)$(RESET)"
endif
ifeq ($(HAS_YOSYS),yes)
	@echo "$(GREEN)yosys found$(RESET)"
else
	@echo "$(YELLOW)yosys not found (optional)$(RESET)"
endif

# Project information
.PHONY: info
info:
	@echo "$(BOLD)$(BLUE)C++ Project Information:$(RESET)"
	@echo ""
	@echo "$(BOLD)Project:$(RESET)     $(PROJECT_NAME)"
	@echo "$(BOLD)Platform:$(RESET)    $(PLATFORM)"
	@echo "$(BOLD)Build Type:$(RESET)  $(BUILD_TYPE)"
	@echo "$(BOLD)Generator:$(RESET)   $(CMAKE_GENERATOR)"
	@echo "$(BOLD)Language:$(RESET)    C++23"
	@echo ""
	@echo "$(BOLD)$(GREEN)Directories:$(RESET)"
	@echo "  Source:      $(SRC_DIR)/"
	@echo "  Headers:     $(INCLUDE_DIR)/"
	@echo "  Tests:       $(TESTS_DIR)/"
	@echo "  Build:       $(BUILD_DIR)/"
	@echo "  Binaries:    $(BIN_DIR)/"
	@echo "  Libraries:   $(LIB_DIR)/"
	@echo "  Simulation:  $(SIM_DIR)/"
	@echo ""
	@find $(SRC_DIR) -name "*.cpp" 2>$(NULL_DEVICE) | wc -l | xargs echo "C++ source files:"
	@find $(INCLUDE_DIR) -name "*.hpp" -o -name "*.h" 2>$(NULL_DEVICE) | wc -l | xargs echo "Header files:"
	@find $(SIM_DIR) -name "*.v" 2>$(NULL_DEVICE) | wc -l | xargs echo "Verilog files:"

# Watch for changes (Linux/macOS with inotify-tools)
.PHONY: watch
watch:
ifneq ($(PLATFORM),Windows)
	@if command -v inotifywait >$(NULL_DEVICE) 2>&1; then \
		echo "$(BOLD)$(BLUE)Watching for changes... (Ctrl+C to stop)$(RESET)"; \
		while true; do \
			inotifywait -q -r -e modify,create,delete $(SRC_DIR) $(INCLUDE_DIR) && \
			$(MAKE) build; \
		done; \
	else \
		echo "$(YELLOW)inotifywait not found. Install inotify-tools for watch functionality.$(RESET)"; \
	fi
else
	@echo "$(YELLOW)Watch functionality not implemented for Windows$(RESET)"
endif

# Documentation target
.PHONY: docs
docs:
	@echo "$(BOLD)$(BLUE)Generating documentation...$(RESET)"
	@if command -v doxygen >$(NULL_DEVICE) 2>&1; then \
		doxygen Doxyfile; \
		echo "$(GREEN)Documentation generated in docs/html/$(RESET)"; \
	else \
		echo "$(RED)doxygen not found. Install with: sudo apt install doxygen graphviz$(RESET)"; \
	fi

# Open documentation in browser
.PHONY: open-docs
open-docs:
	@if [ -f docs/html/index.html ]; then \
		echo "$(BOLD)$(BLUE)Opening documentation...$(RESET)"; \
		case "$(PLATFORM)" in \
			macOS)   open docs/html/index.html ;; \
			Windows) start docs/html/index.html ;; \
			*)       xdg-open docs/html/index.html ;; \
		esac; \
	else \
		echo "$(RED)docs/html/index.html not found. Run 'make docs' first.$(RESET)"; \
	fi

# Benchmark target (placeholder)
.PHONY: benchmark
benchmark: release
	@echo "$(BOLD)$(BLUE)Running performance benchmarks...$(RESET)"
	@echo "$(YELLOW)Benchmark implementation needed$(RESET)"

# ─── Verilog targets ───────────────────────────────────────────────────────────

# Compile and simulate Verilog with iverilog
.PHONY: sim
sim:
ifeq ($(HAS_IVERILOG),yes)
	@echo "$(BOLD)$(BLUE)Compiling Verilog sources...$(RESET)"
	@$(MKDIR) $(BUILD_DIR)
	@iverilog -o $(VVP_OUT) -g2012 $(VERILOG_SRC)
	@echo "$(BOLD)$(BLUE)Running simulation...$(RESET)"
	@vvp $(VVP_OUT)
	@echo "$(GREEN)Simulation complete!$(RESET)"
else
	@echo "$(RED)iverilog not found.$(RESET)"
ifeq ($(PLATFORM),Linux)
	@echo "Install with: sudo apt install iverilog"
else ifeq ($(PLATFORM),macOS)
	@echo "Install with: brew install icarus-verilog"
else
	@echo "Install via MSYS2: pacman -S mingw-w64-x86_64-iverilog"
endif
endif

# Open waveform in GTKWave
.PHONY: wave
wave: sim
ifeq ($(HAS_GTKWAVE),yes)
	@if [ -f $(VCD_OUT) ]; then \
		echo "$(BOLD)$(BLUE)Opening waveform in GTKWave...$(RESET)"; \
		gtkwave $(VCD_OUT); \
	else \
		echo "$(YELLOW)No VCD file found at $(VCD_OUT). Ensure your testbench dumps a VCD.$(RESET)"; \
	fi
else
	@echo "$(RED)gtkwave not found.$(RESET)"
ifeq ($(PLATFORM),Linux)
	@echo "Install with: sudo apt install gtkwave"
else ifeq ($(PLATFORM),macOS)
	@echo "Install with: brew install --cask gtkwave"
else
	@echo "Install via MSYS2: pacman -S mingw-w64-x86_64-gtkwave"
endif
endif

# Lint Verilog with Verilator
.PHONY: lint-verilog
lint-verilog:
ifeq ($(HAS_VERILATOR),yes)
	@echo "$(BOLD)$(BLUE)Linting Verilog with Verilator...$(RESET)"
	@verilator --lint-only -Wall $(VERILOG_SRC)
	@echo "$(GREEN)Verilog lint complete!$(RESET)"
else
	@echo "$(RED)verilator not found.$(RESET)"
ifeq ($(PLATFORM),Linux)
	@echo "Install with: sudo apt install verilator"
else ifeq ($(PLATFORM),macOS)
	@echo "Install with: brew install verilator"
else
	@echo "See https://verilator.org for Windows installation"
endif
endif

# Synthesize with Yosys
.PHONY: synth
synth:
ifeq ($(HAS_YOSYS),yes)
	@echo "$(BOLD)$(BLUE)Synthesizing $(VERILOG_TOP) with Yosys...$(RESET)"
	@$(MKDIR) $(BUILD_DIR)
	@yosys -p "read_verilog $(VERILOG_SRC); synth -top $(VERILOG_TOP); write_json $(BUILD_DIR)/$(VERILOG_TOP).json"
	@echo "$(GREEN)Synthesis complete! Output: $(BUILD_DIR)/$(VERILOG_TOP).json$(RESET)"
else
	@echo "$(RED)yosys not found.$(RESET)"
ifeq ($(PLATFORM),Linux)
	@echo "Install with: sudo apt install yosys"
else ifeq ($(PLATFORM),macOS)
	@echo "Install with: brew install yosys"
else
	@echo "See https://yosyshq.net for Windows installation"
endif
endif

# Utility targets
.PHONY: check-cmake
check-cmake:
ifneq ($(HAS_CMAKE),yes)
	@echo "$(RED)CMake not found. Please install CMake 3.14 or later.$(RESET)"
	@exit 1
endif

