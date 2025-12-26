# Makefile wrapper for common SneezyMUD build tasks
# This is a convenience wrapper around CMake commands

.PHONY: help build release clean clean-all format run test reconfigure report all
.PHONY: iwyu-check iwyu-fix
.PHONY: analyze analyze-export analyze-ci
.PHONY: check-python3

# Default target
all: build

# Default preset
PRESET ?= dev

# Sanitizer options for development runs
export ASAN_OPTIONS := strict_string_checks=1:detect_stack_use_after_return=1:check_initialization_order=1:strict_init_order=1
export UBSAN_OPTIONS := print_stacktrace=1:halt_on_error=1

help:
	@echo "SneezyMUD Build Commands:"
	@echo ""
	@echo "  make build       - Build debug version with sanitizers (default)"
	@echo "  make release     - Build optimized release version"
	@echo "  make clean       - Remove build directory"
	@echo "  make clean-all   - Remove build directory and analysis results"
	@echo "  make format      - Run clang-format on all source files"
	@echo "  make run         - Build and run the server on port 4000"
	@echo "  make report      - Clean build with error/warning report"
	@echo "  make reconfigure - Reconfigure CMake (dev preset)"
	@echo "  make iwyu-check  - Run include-what-you-use analysis (dry run)"
	@echo "  make iwyu-fix    - Run include-what-you-use and apply fixes"
	@echo ""
	@echo "Static Analysis (CodeChecker + clang-tidy + clangsa):"
	@echo "  make analyze        - Run comprehensive static analysis"
	@echo "  make analyze-export - Export results (FMT=html|text|sqlite)"
	@echo "  make analyze-ci     - CI gating (exits non-zero if critical issues)"
	@echo ""
	@echo "Build options:"
	@echo "  PORT=<number>   - Run server on custom port (use with 'make run')"
	@echo "  FMT=<format>    - Export format: html, text, or sqlite (default: html)"
	@echo "  CRITICAL=1      - Filter to critical issues only"
	@echo "  OUTPUT=<path>   - Output path for export"
	@echo ""
	@echo "Examples:"
	@echo "  make build                              - Build the server"
	@echo "  make run                                - Build and run on default port 4000"
	@echo "  make report                             - See all warnings/errors in the codebase"
	@echo "  make analyze                            - Run comprehensive static analysis"
	@echo "  make analyze-export                     - Full HTML report"
	@echo "  make analyze-export FMT=text CRITICAL=1 - Critical issues as text"
	@echo "  make PORT=5000 run                      - Build and run on port 5000"

# Development build (default preset)
build:
	@if [ ! -d build ]; then \
		echo "Configuring build (dev preset)..."; \
		cmake --preset $(PRESET); \
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

# Clean build directory and analysis results
clean-all:
	@echo "Removing build directory and analysis results..."
	@rm -rf build .codechecker-results codechecker-report compile_commands.json
	@echo "Clean complete"

# Reconfigure CMake
reconfigure:
	@echo "Reconfiguring CMake (dev preset)..."
	@cmake --preset $(PRESET)

# Format code
format:
	@if [ ! -d build ]; then \
		echo "Build directory not found. Configuring first..."; \
		cmake --preset $(PRESET); \
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

# =============================================================================
# Static Analysis via CodeChecker (Python script)
# =============================================================================
# All analysis logic is in scripts/analyze.py for clarity and maintainability.
#
# Workflow:
#   1. make analyze                  - Run full analysis
#   2. make analyze-export FMT=X     - Export results of previous run in desired format
#   3. make analyze-ci               - CI gating (exits non-zero if critical issues)
#
# The Python script handles:
#   - clang-tidy: 15 production-critical AST checks (same as .clang-tidy WarningsAsErrors)
#   - clangsa: All checks including experimental alpha (deep mode + CTU)
#   - Filtering to critical-only issues
#   - Multiple output formats (html, text, sqlite) - all default to codechecker-report/

PYTHON := python3
ANALYZE := $(PYTHON) scripts/analyze.py

check-python3:
	@command -v $(PYTHON) >/dev/null 2>&1 || { \
		echo "Error: 'python3' not found."; \
		echo "Install with: apt install python3"; \
		exit 1; \
	}

# Run comprehensive static analysis
analyze: build check-python3
	$(ANALYZE) run --preset $(PRESET)

# Unified export target with parameters
# Usage: make analyze-export [FMT=html|text|sqlite] [CRITICAL=1] [OUTPUT=path]
# Examples:
#   make analyze-export                     # HTML report (codechecker-report/)
#   make analyze-export FMT=text            # Text (codechecker-report/issues.txt)
#   make analyze-export FMT=sqlite          # SQLite (codechecker-report/issues.db)
#   make analyze-export FMT=text CRITICAL=1 # Critical-only text
FMT ?= html
analyze-export: check-python3
	$(ANALYZE) export --format $(FMT) $(if $(CRITICAL),--critical-only) $(if $(OUTPUT),--output $(OUTPUT))

# CI gating - exits non-zero if critical violations found
analyze-ci: check-python3
	$(ANALYZE) critical --exit-code
