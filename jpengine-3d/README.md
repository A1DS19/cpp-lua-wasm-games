# jpengine-3d

Modern C++23 project with cross-platform support and professional development tooling.

## Features

- **Modern C++23** features
- **Cross-platform** support (Windows, macOS, Linux)
- **CMake** build system
- **LSP support** via clangd
- **Code formatting** with clang-format
- **Unit testing** framework ready

## Quick Start

### Prerequisites

- **CMake 3.25+**
- **C++23 compatible compiler:**
  - GCC 13+ (Linux/Windows)
  - Clang 15+ (macOS/Linux)
  - MSVC 2022 (Windows)

### Building

```bash
cd jpengine-3d
make setup-lsp  # Setup LSP for your editor (recommended)
make build      # or just 'make'
make run
```

## Development

### Available Make Targets

**Building**
```bash
make build      # Configure CMake and build the project (default)
make rebuild    # Clean then build from scratch
make release    # Build optimized release binary (O2, no debug symbols)
make install    # Install binary to /usr/local/bin (requires sudo)
make uninstall  # Remove installed binary (requires sudo)
```

**Running & Testing**
```bash
make run        # Build and run the main executable
make test       # Build and run the test suite
make debug      # Build and launch with gdb (Linux) or lldb (macOS)
make valgrind   # Run under Valgrind for memory error detection
make memcheck   # Comprehensive memory analysis (Valgrind or scripts/memcheck.sh)
```

**Code Quality**
```bash
make format     # Auto-format all .cpp/.hpp/.h files with clang-format
make analyze    # Run static analysis (scripts/analyze.sh)
make lint       # Alias for analyze
make coverage   # Generate code coverage report (scripts/coverage.sh)
make setup-lsp  # Write .clangd config for editor LSP support
```

**Verilog**
```bash
make sim              # Compile and simulate .v files in sim/ with iverilog
make wave             # Run sim then open the VCD waveform in GTKWave
make lint-verilog     # Lint Verilog sources with Verilator (--lint-only -Wall)
make synth            # Synthesize with Yosys, outputs build/<top>.json
```
Override the top module: `make sim VERILOG_TOP=mymodule`

**Maintenance**
```bash
make clean      # Remove build artifacts, keep directory structure
make distclean  # Full clean including CMake cache (forces full reconfigure)
make deps       # Show required and optional build dependencies
make check      # Detect installed tools and report what is/isn't found
make info       # Show project name, platform, build type, file counts
```

**Development**
```bash
make watch      # Auto-rebuild when source files change (requires inotify-tools)
make benchmark  # Build release and run benchmarks (implement in scripts/)
make docs       # Generate HTML API docs with Doxygen (also runs on build)
make open-docs  # Open docs/html/index.html in the default browser
make help       # Print this target list with short descriptions
```

### Documentation
```bash
sudo apt install doxygen graphviz
make docs       # Generate HTML docs (also runs automatically on make build)
```
Open `docs/html/index.html` in your browser.

### Using Scripts Directly
```bash
./scripts/build.sh        # Cross-platform build script
./scripts/format.sh       # Format code
./scripts/clean.sh        # Clean build
./scripts/setup_clangd.sh # Regenerate LSP configuration
```

## Platform-Specific Notes

### Linux (Ubuntu 24.04+)
```bash
sudo apt update
sudo apt install build-essential cmake g++-13 clang-format
sudo apt install gdb
```

### macOS
```bash
brew install cmake llvm clang-format
xcode-select --install
```

### Windows
- Install Visual Studio 2022 with C++ workload
- Or install MSYS2/MinGW-w64 for GCC
- Install CMake from cmake.org

## Project Structure

```
jpengine-3d/
├── Makefile           # Cross-platform make targets
├── CMakeLists.txt     # CMake configuration
├── .clangd            # LSP configuration
├── .clang-format      # Code formatting rules
├── .gitignore
│
├── bin/               # Compiled executables
├── build/             # CMake build files
├── lib/               # External libraries
│
├── include/           # Header files (.hpp, .h)
│   └── main.hpp
│
├── src/               # Source files (.cpp, .cxx)
│   └── main.cpp
│
├── tests/             # Test files
│   └── test_main.cpp
│
└── scripts/           # Build and utility scripts
    ├── build.sh
    ├── format.sh
    ├── clean.sh
    └── setup_clangd.sh
```

## Modern C++ Features Used

- C++23 standard library features
- Auto return type deduction
- String views for efficient string handling
- `std::format` for modern string formatting (when available)

## Editor Setup

### LunarVim / Neovim
1. Run `make setup-lsp` to configure clangd
2. LSP should work automatically with proper IntelliSense

### VS Code
1. Install C/C++ extension
2. Run `make setup-lsp`

### CLion / Other IDEs
- Import as CMake project

## Contributing

1. Format code before committing: `make format`
2. Run tests: `make test`

## License

[Add your license here]
