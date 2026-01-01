# Compiler options and warning flags for C++ compilation
# This is a C codebase compiled as C++20. Warnings are tuned to catch real bugs
# while progressively modernizing the codebase.

add_library(compiler_options INTERFACE)
add_library(sneezy::compiler_options ALIAS compiler_options)

# ccache support - auto-enabled if found (speeds up clean rebuilds and branch switching)
# NO_CACHE ensures we always check for ccache, even when reusing a cached CMake configuration
find_program(CCACHE_PROGRAM ccache NO_CACHE)
if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    message(STATUS "Using ccache: ${CCACHE_PROGRAM}")
endif()

# =============================================================================
# Warning flags - common to both GCC and Clang
# =============================================================================
target_compile_options(compiler_options INTERFACE
    # Colored diagnostics for better readability
    -fdiagnostics-color=always

    # Foundation warnings
    -Wall                           # Enable most warning messages
    -Wextra                         # Enable extra warning messages
    -Wpedantic                      # Strict ISO C++ compliance warnings

    # Type safety
    -Wconversion                    # Implicit conversions that may change a value
    -Wsign-conversion               # Sign-related type mismatches
    -Wdouble-promotion              # Float implicitly promoted to double
    -Wcast-align                    # Pointer casts that increase alignment
    -Wcast-qual                     # Casts that remove type qualifiers

    # Format strings
    -Wformat=2                      # Strict format string checking
    -Wformat-security               # Printf/scanf security issues
    -Wno-format-nonliteral          # Disabled - variadic format wrappers used extensively

    # Code quality
    -Wshadow                        # Variable shadows another variable
    -Wundef                         # Undefined identifier in #if
    -Wuninitialized                 # Uninitialized automatic variables
    -Wredundant-decls               # Multiple declarations of same object
    -Wswitch-default                # Switch has no default case
    -Wswitch-enum                   # Switch on enum doesn't handle all values
    -Wpointer-arith                 # Sizeof void and function pointer arithmetic
    -Wwrite-strings                 # String literals written through non-const pointers
    -Wdisabled-optimization         # Requested optimization pass is disabled
    -Wunknown-pragmas               # Unknown #pragma directives
    -Wstring-compare                # Suspicious string comparisons

    # C++ specific
    -Wnon-virtual-dtor              # Class has virtual functions but no virtual destructor
    -Woverloaded-virtual            # Overloaded virtual function name hides parent implementation
    -Wold-style-cast                # C-style casts (many in legacy code - fix over time)
    -Wzero-as-null-pointer-constant # 0 used as null pointer (prefer nullptr)
    -Wsuggest-override              # Missing override on virtual methods
    -Wextra-semi                    # Extra semicolons outside of function bodies
    -Wmissing-declarations          # Global functions without prior declarations

    # Functions
    -Winline                        # Inline function cannot be inlined

    # Other
    -Wdate-time                     # __TIME__, __DATE__, __TIMESTAMP__ usage
    -Wmissing-include-dirs          # Missing user-specified include directories
    -Wvla                           # Variable-length arrays
    -Wshift-overflow                # Shift overflows
    -Wdeprecated                    # Deprecated features
)

# =============================================================================
# Clang-specific warnings
# =============================================================================
target_compile_options(compiler_options INTERFACE
    $<$<CXX_COMPILER_ID:Clang>:
        -ferror-limit=0                 # Don't limit number of errors shown

        # C++20 compatibility
        -Wc++20-compat-pedantic         # C++20 compatibility issues
        -Wc++20-extensions              # Non-standard C++20 extensions

        # Uninitialized/null detection
        -Wconditional-uninitialized     # Better uninitialized detection than GCC
        -Wnull-dereference              # Potential null pointer dereference

        # Loop and control flow
        -Wloop-analysis                 # Loop variable misuse (double increment, etc.)
        -Wimplicit-fallthrough          # Missing fallthrough in switch
        -Wrange-loop-construct          # Range loop inefficiencies

        # Type and enum safety
        -Wshift-sign-overflow           # Left shift overflows into sign bit
        -Wenum-enum-conversion          # Enum-to-enum conversions
        -Wenum-float-conversion         # Enum-to-float conversions
        -Wtautological-compare          # Comparisons always true/false
        -Wctad-maybe-unsupported        # Class template argument deduction issues

        # Arrays and bounds
        -Warray-bounds                  # Array bounds violations

        # Format strings
        -Wformat-type-confusion         # Format string type mismatches
        -Wformat-non-iso                # Non-ISO format strings
        -Wformat-pedantic               # Pedantic format string checking
    >
)

# =============================================================================
# GCC-specific warnings
# =============================================================================
target_compile_options(compiler_options INTERFACE
    $<$<CXX_COMPILER_ID:GNU>:
        -fmax-errors=0                  # Don't limit number of errors shown

        # Code analysis
        -Wlogical-op                    # Suspicious logical operations
        -Wduplicated-cond               # Duplicated conditions in if-else chains
        -Wduplicated-branches           # Duplicated branches in if-else chains
        -Wnull-dereference              # Potential null pointer dereference

        # Arrays and bounds
        -Warray-bounds=2                # More aggressive array bounds checking

        # Control flow
        -Wimplicit-fallthrough=5        # Strict fallthrough checking

        # Format strings
        -Wformat-overflow=2             # Printf buffer overflow detection
        -Wformat-truncation=2           # Printf truncation detection

        # Loop optimization
        -funsafe-loop-optimizations     # Enable unsafe loop optimizations
        -Wunsafe-loop-optimizations     # Warn when unsafe loop optimizations applied

        # Static analysis
        -Wanalyzer-possible-null-dereference  # GCC static analyzer null checks

        # C++ specific
        -Wuseless-cast                  # Unnecessary casts
    >
)

# =============================================================================
# Debug options - for better debugging and sanitizer stack traces
# =============================================================================
target_compile_options(compiler_options INTERFACE
    $<$<CONFIG:Debug>:
        -O0                             # No optimization for debugging
    >
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:
        -fno-common                     # Stricter symbol handling
        -fno-optimize-sibling-calls     # Don't optimize tail calls (better traces)
        -fno-omit-frame-pointer         # Keep frame pointers for clear stack traces
    >
    # GCC/GDB optimized debug info
    $<$<AND:$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>,$<CXX_COMPILER_ID:GNU>>:
        -ggdb3                          # Maximum debug info for GDB
    >
    # Clang/LLDB optimized debug info
    $<$<AND:$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>,$<CXX_COMPILER_ID:Clang>>:
        -glldb                          # Clang/LLDB optimized debug info
        -fno-limit-debug-info           # Full debug info (not limited)
    >
)

# Debug symbols for linker
target_link_options(compiler_options INTERFACE
    $<$<AND:$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>,$<CXX_COMPILER_ID:GNU>>:
        -ggdb3
    >
    $<$<AND:$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>,$<CXX_COMPILER_ID:Clang>>:
        -glldb
    >
)

# =============================================================================
# Optimization levels based on build type
# =============================================================================
target_compile_options(compiler_options INTERFACE
    $<$<CONFIG:Release>:-O2>        # Standard optimization for release
)

# Enable LTO for release builds (interprocedural optimization)
target_compile_options(compiler_options INTERFACE
    $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:Clang>>:-flto=thin>
    $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:GNU>>:-flto=auto>
)
target_link_options(compiler_options INTERFACE
    $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:Clang>>:-flto=thin>
    $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:GNU>>:-flto=auto>
)

# =============================================================================
# Dead code detection - prints unused functions at link time
# Usage: cmake --preset dev -DENABLE_DEAD_CODE_DETECTION=ON
# =============================================================================
option(ENABLE_DEAD_CODE_DETECTION "Print unused functions during linking" OFF)
if(ENABLE_DEAD_CODE_DETECTION)
    target_compile_options(compiler_options INTERFACE
        -ffunction-sections             # Put each function in its own section
        -fdata-sections                 # Put each data item in its own section
    )
    target_link_options(compiler_options INTERFACE
        -Wl,--gc-sections               # Remove unused sections
        -Wl,--print-gc-sections         # Print what was removed
    )
    message(STATUS "Dead code detection enabled - unused functions will be printed at link time")
endif()

# =============================================================================
# Include-What-You-Use (IWYU) support
# =============================================================================

# IWYU fix target - runs IWYU analysis and automatically applies fixes
# Usage: cmake --build build --target iwyu-fix
find_program(IWYU_TOOL iwyu_tool.py)
find_program(FIX_INCLUDES fix_includes.py)
if(IWYU_TOOL AND FIX_INCLUDES)
    # IWYU options:
    #   --cxx17ns: C++17 nested namespace syntax for forward declarations
    # Note: --no_comments removes line numbers needed by fix_includes.py, so only use for check
    set(IWYU_ARGS_BASE "-Xiwyu;--cxx17ns")

    # Project-specific mappings (if present)
    if(EXISTS "${CMAKE_SOURCE_DIR}/iwyu.imp")
        list(APPEND IWYU_ARGS_BASE "-Xiwyu;--mapping_file=${CMAKE_SOURCE_DIR}/iwyu.imp")
    endif()

    # Fix target needs line numbers in output (no --no_comments)
    string(REPLACE ";" " " IWYU_ARGS_FIX "${IWYU_ARGS_BASE}")

    # Check target can use cleaner output without line number comments
    set(IWYU_ARGS_CHECK "${IWYU_ARGS_BASE};-Xiwyu;--no_comments")
    string(REPLACE ";" " " IWYU_ARGS_CHECK "${IWYU_ARGS_CHECK}")

    add_custom_target(iwyu-fix
        COMMAND ${CMAKE_COMMAND} -E echo "Running IWYU analysis and applying fixes..."
        COMMAND sh -c "${IWYU_TOOL} -j 0 -p ${CMAKE_BINARY_DIR} -- ${IWYU_ARGS_FIX} 2>&1 | ${FIX_INCLUDES}"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running include-what-you-use and applying fixes"
        VERBATIM
    )

    add_custom_target(iwyu-check
        COMMAND ${CMAKE_COMMAND} -E echo "Running IWYU analysis (dry run)..."
        COMMAND sh -c "${IWYU_TOOL} -j 0 -p ${CMAKE_BINARY_DIR} -- ${IWYU_ARGS_CHECK}"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running include-what-you-use (analysis only, no fixes applied)"
        VERBATIM
    )
    message(STATUS "IWYU targets available: iwyu-fix, iwyu-check")
endif()
