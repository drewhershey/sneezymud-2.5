# Compiler options and warning flags for Clang
add_library(compiler_options INTERFACE)
add_library(sneezy::compiler_options ALIAS compiler_options)

# Warning flags
target_compile_options(compiler_options INTERFACE
    # Foundation warnings
    -Wall                           # Enable most warning messages
    -Wextra                         # Enable extra warning messages
    -Wpedantic                      # Strict ISO C compliance warnings

    # Type safety
    -Wconversion                    # Implicit conversions that may change a value
    -Wsign-conversion               # Sign-related type mismatches
    -Wdouble-promotion              # Float implicitly promoted to double
    -Wcast-align                    # Pointer casts that increase alignment
    -Wcast-qual                     # Casts that remove type qualifiers

    # Format strings
    -Wformat=2                      # Strict format string checking
    -Wformat-security               # Printf/scanf security issues
    -Wno-format-nonliteral          # Disabled - pattern used extensively in codebase

    # Functions
    -Wmissing-declarations          # Global functions without previous declarations
    -Wmissing-prototypes            # Global functions without prototypes
    -Wstrict-prototypes             # Functions declared without argument types
    -Wold-style-definition          # Old-style function definitions
    -Wnested-externs                # 'extern' declarations inside functions
    -Wbad-function-cast             # Casting functions to incompatible types

    # Code quality
    -Wshadow                        # Variable shadows another variable
    -Wundef                         # Undefined identifier in #if
    -Wuninitialized                 # Uninitialized automatic variables
    -Wredundant-decls               # Multiple declarations of same object
    -Wswitch-default                # Switch has no default case
    -Wswitch-enum                   # Switch on enum doesn't handle all values
    -Wpointer-arith                 # Sizeof void and function pointer arithmetic
    -Wwrite-strings                 # String literals written through non-const pointers

    # Clang-specific warnings (modern best practices)
    -Wconditional-uninitialized     # Better uninitialized detection than GCC
    -Wloop-analysis                 # Loop variable misuse (double increment, etc.)
    -Wshift-sign-overflow           # Left shift overflows into sign bit
    -Wenum-enum-conversion          # Enum-to-enum conversions
    -Wenum-float-conversion         # Enum-to-float conversions
    -Wtautological-compare          # Comparisons always true/false
    -Wimplicit-fallthrough          # Missing fallthrough in switch
    -Warray-bounds                  # Array bounds violations
    -Wnull-dereference              # Potential null pointer dereference

    # Other
    -Waggregate-return              # Returning structures (may cause inefficiencies)
    -Wc++-compat                    # C++ compatibility issues
    -Wdate-time                     # __TIME__, __DATE__, __TIMESTAMP__ usage
    -Winline                        # Inline function cannot be inlined
    -Wmissing-include-dirs          # Missing user-specified include directories
    -Wvla                           # Variable-length arrays
    -Wshift-overflow                # Shift overflows
    -ferror-limit=0                 # Don't limit number of errors shown
)

# Always include full debug symbols for sanitizer stack traces
target_compile_options(compiler_options INTERFACE
    -glldb                          # Full debug symbols (clang/lldb optimized)
    -fno-omit-frame-pointer         # Keep frame pointers for clear stack traces
    -fno-optimize-sibling-calls     # Don't optimize tail calls (better traces)
    -fno-common                     # Stricter symbol handling
)

# Optimization levels based on build type
target_compile_options(compiler_options INTERFACE
    $<$<CONFIG:Debug>:-O0>          # No optimization for debugging
    $<$<CONFIG:Release>:-O2>        # Standard optimization for release
)
