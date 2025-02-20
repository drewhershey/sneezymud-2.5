# Ensure bash is used for shell commands
SHELL := /bin/bash

# Enable parallel builds by default (uses number of CPU cores automatically)
MAKEFLAGS += -j

# Environment variables for sanitizers
export ASAN_OPTIONS := halt_on_error=1:detect_stack_use_after_return=1:check_initialization_order=1:strict_string_checks=1:detect_invalid_pointer_pairs=2
export UBSAN_OPTIONS := print_stacktrace=1:halt_on_error=1:print_summary=1

# Project configuration
NAME := sneezy
CC := /usr/bin/clang

# Build options - override with 'make OPTION=value'
GPROF ?= 0     # Enable gprof profiling
OLEVEL ?= 0     # Optimization level

#  Feature Flags
#
# -DHASH          : Use hash tables for rooms (More CPU, less storage)
# -DNOTRACK       : Disable tracking feature (lower CPU usage)
# -DLIMITED_ITEMS : Limit items above specific rent cost
# -DPLAYER_AUTH   : Force new players to be authorized
# -DDEBUG         : Enable debugging features
# -DSITELOCK      : Enable host lockout commands
# -DNODUPLICATES  : Prevent item duplication exploits
DEFINES += -DNODUPLICATES

WARNINGS := -Wall -Wextra

# Base flags
CFLAGS := -std=c17 $(WARNINGS) $(DEFINES) # Use C17 standard
CFLAGS += -O$(OLEVEL) # Optimization level
CFLAGS += -MMD -MP # Generate dependency files to enable incremental builds
CFLAGS += -fno-common # Stricter symbol handling
CFLAGS += -ggdb3 # Add all possible debugging information, specifically for GDB
CFLAGS += -fno-omit-frame-pointer # Keep frame pointers for better stack traces
CFLAGS += -fno-optimize-sibling-calls # Keep sibling calls for better stack traces
CFLAGS += -fsanitize=address,undefined,leak # Enable sanitizers for runtime checks

LDFLAGS := -lcrypt # Link withCrypt library
LDFLAGS += -fsanitize=address,undefined,leak # Link with sanitizers

# Add gprof mapping, if enabled above
ifeq ($(GPROF),1)
		CFLAGS += -pg
		LDFLAGS += -pg
endif

# Directories
SRCDIR := code
OBJDIR := $(SRCDIR)/objs
DEPDIR := $(SRCDIR)/deps

# Find all source files
SRCS := $(wildcard code/*.c)

# Generate object file list
OBJS := $(sort $(SRCS:code/%.c=$(OBJDIR)/%.o))

# Generate list of dependency files
DEPS := $(OBJS:.o=.d)

# Include generated dependency files (if they exist)
-include $(DEPS)

# Phony targets
.PHONY: clean help format

# Main build target
$(NAME): $(OBJS)
		@echo "Linking $@..."
		@$(CC) $(OBJS) $(LDFLAGS) -o $@
		@echo "Build complete: $@"
		@file $@
		@echo "Timestamp: $$(date)"

# Create build directories
$(OBJDIR):
		@mkdir -p $@
		@echo "Created directory: $@"

# Object file compilation
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
		@echo "Compiling $<..."
		$(CC) $(CFLAGS) -c $< -o $@

# Format source files (requires clang-format)
format:
		@echo "Formatting source files..."
		find $(SRCDIR) -name '*.[ch]' -exec clang-format -i {} +

# Clean build output
clean:
		@echo "Cleaning build output..."
		rm -rf $(OBJDIR) $(NAME)
		@echo "Clean complete"

# Help target
help:
		@echo "Available targets:"
		@echo "	$(NAME)                  - Build the project (default)"
		@echo "	clean                   - Remove build output"
		@echo "	format                  - Format source files"
		@echo "	help                    - Show this help message"
		@echo ""
		@echo "Build options:"
		@echo "	make GPROF=0|1          - Toggle gprof profiling [0 | 1] (default: 0)"
		@echo "	make OLEVEL=0|1|2|3|g   - Set optimization level [0 | 1 | 2 | 3 | g] (default: 0)"
