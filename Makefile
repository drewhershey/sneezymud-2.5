# Makefile wrapper for common SneezyMUD build tasks
# This is a convenience wrapper around CMake commands

.PHONY: help build release clean format run test reconfigure report all iwyu-check iwyu-fix

# Default target
all: build

help:
	@echo "SneezyMUD Build Commands:"
	@echo ""
	@echo "  make build      - Build debug version with sanitizers (default)"
	@echo "  make release    - Build optimized release version"
	@echo "  make clean      - Remove build directory"
	@echo "  make format     - Run clang-format on all source files"
	@echo "  make run        - Build and run the server on port 4000"
	@echo "  make report     - Clean build with error/warning report"
	@echo "  make reconfigure - Reconfigure CMake (dev preset)"
	@echo "  make iwyu-check - Run include-what-you-use analysis (dry run)"
	@echo "  make iwyu-fix   - Run include-what-you-use and apply fixes"
	@echo ""
	@echo "Build options:"
	@echo "  PORT=<number>   - Run server on custom port (use with 'make run')"
	@echo ""
	@echo "Examples:"
	@echo "  make build      - Build the server"
	@echo "  make run        - Build and run on default port 4000"
	@echo "  make report     - See all warnings/errors in the codebase"
	@echo "  make PORT=5000 run - Build and run on port 5000"

# Development build (default preset)
build:
	@if [ ! -d build ]; then \
		echo "Configuring build (dev preset)..."; \
		cmake --preset dev; \
	fi
	@echo "Building..."
	@cmake --build build

# Release build
release:
	@echo "Configuring release build..."
	@cmake --preset release
	@echo "Building release..."
	@cmake --build build

# Clean build directory
clean:
	@echo "Removing build directory..."
	@rm -rf build
	@rm -f compile_commands.json
	@echo "Clean complete"

# Reconfigure CMake
reconfigure:
	@echo "Reconfiguring CMake (dev preset)..."
	@cmake --preset dev

# Format code
format:
	@if [ ! -d build ]; then \
		echo "Build directory not found. Configuring first..."; \
		cmake --preset dev; \
	fi
	@echo "Formatting code..."
	@cmake --build build --target format

# Run the server (build first)
PORT ?= 4000
run: build
	@echo "Starting server on port $(PORT)..."
	@./sneezy $(PORT)

# Clean build with error/warning report
report:
	@./scripts/build-report.sh

# Run IWYU analysis (dry run, no changes)
iwyu-check: build
	@echo "Running include-what-you-use analysis..."
	@cmake --build build --target iwyu-check

# Run IWYU analysis and apply fixes
iwyu-fix: build
	@echo "Running include-what-you-use and applying fixes..."
	@cmake --build build --target iwyu-fix
