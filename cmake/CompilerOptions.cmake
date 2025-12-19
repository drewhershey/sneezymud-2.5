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

    # C23-specific warnings
    -Wc23-extensions                # C23 features used in older standard modes
    -Wdeprecated-attributes         # Deprecated attribute usage
    -Wbitfield-width                # Suspicious bitfield widths
    -Wformat-type-confusion         # Format string type mismatches
    -Wnullability-completeness      # Incomplete nullability annotations
    -Wzero-as-null-pointer-constant # 0 used as null pointer (prefer nullptr)

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

# Enable LTO for release builds (interprocedural optimization)
target_compile_options(compiler_options INTERFACE
    $<$<CONFIG:Release>:-flto=auto>
)
target_link_options(compiler_options INTERFACE
    $<$<CONFIG:Release>:-flto=auto>
)

# Include-What-You-Use (IWYU) - optional analysis tool for header cleanup
# Usage: cmake --preset dev -DENABLE_IWYU=ON
option(ENABLE_IWYU "Run include-what-you-use during build (for header analysis)" OFF)
if(ENABLE_IWYU)
    find_program(IWYU_PROGRAM include-what-you-use)
    if(IWYU_PROGRAM)
        set(CMAKE_C_INCLUDE_WHAT_YOU_USE "${IWYU_PROGRAM}")
        message(STATUS "IWYU enabled: ${IWYU_PROGRAM}")
    else()
        message(WARNING "IWYU requested but include-what-you-use not found")
    endif()
endif()

# IWYU fix target - runs IWYU analysis and automatically applies fixes
# Usage: cmake --build build --target iwyu-fix
find_program(IWYU_TOOL iwyu_tool.py)
find_program(FIX_INCLUDES fix_includes.py)
if(IWYU_TOOL AND FIX_INCLUDES)
    # Find IWYU's mapping files directory (prefer source repo, fall back to installed)
    set(IWYU_MAPPING_DIRS
        "$ENV{HOME}/source/repos/include-what-you-use"
        "/usr/local/share/include-what-you-use"
        "/usr/share/include-what-you-use"
    )
    set(IWYU_DIR "")
    foreach(dir ${IWYU_MAPPING_DIRS})
        if(EXISTS "${dir}/iwyu.gcc.imp")
            set(IWYU_DIR "${dir}")
            break()
        endif()
    endforeach()

    # IWYU options for C code:
    # Note: --no_comments removes line numbers needed by fix_includes.py, so only use for check

    # Project-specific mappings (if present) - loaded first for overrides
    if(EXISTS "${CMAKE_SOURCE_DIR}/iwyu.imp")
        list(APPEND IWYU_ARGS_BASE "-Xiwyu;--mapping_file=${CMAKE_SOURCE_DIR}/iwyu.imp")
    endif()

    if(IWYU_DIR)
        # GCC/glibc mappings for C standard library
        # Note: For C projects, we use gcc.libc.imp and stl.c.headers.imp
        if(EXISTS "${IWYU_DIR}/gcc.libc.imp")
            list(APPEND IWYU_ARGS_BASE "-Xiwyu;--mapping_file=${IWYU_DIR}/gcc.libc.imp")
        endif()
        if(EXISTS "${IWYU_DIR}/stl.c.headers.imp")
            list(APPEND IWYU_ARGS_BASE "-Xiwyu;--mapping_file=${IWYU_DIR}/stl.c.headers.imp")
        endif()
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
