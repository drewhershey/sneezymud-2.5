cmake_minimum_required(VERSION 3.20)

# Prevent in-source builds
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "In-source builds not allowed. Use: cmake -B build")
endif()

project(sneezy LANGUAGES C)

# C standard
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)

# Options
option(ENABLE_SANITIZERS "Enable ASan/UBSan/LeakSan" ON)
option(ENABLE_CCACHE "Use ccache if available" ON)
option(ENABLE_NODUPLICATES "Define NODUPLICATES" ON)

# Optional game feature defines
option(ENABLE_HASH "Use hash tables for rooms (more CPU, less storage)" OFF)
option(ENABLE_NOTRACK "Disable tracking feature (lower CPU usage)" OFF)
option(ENABLE_LIMITED_ITEMS "Limit items above specific rent cost" OFF)
option(ENABLE_DEBUG "Enable debugging features" OFF)
option(ENABLE_SITELOCK "Enable host lockout commands" OFF)

# Include modular cmake files
include(cmake/Toolchain-Clang.cmake)
include(cmake/CompilerOptions.cmake)
include(cmake/Sanitizers.cmake)

# ccache support
if(ENABLE_CCACHE)
    find_program(CCACHE_PROGRAM ccache)
    if(CCACHE_PROGRAM)
        set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
        message(STATUS "Using ccache: ${CCACHE_PROGRAM}")
    endif()
endif()

# Collect sources
file(GLOB SOURCES code/*.c)

# Create executable
add_executable(sneezy ${SOURCES})
target_include_directories(sneezy PRIVATE code)
target_link_libraries(sneezy PRIVATE crypt)

# Apply compiler options
target_link_libraries(sneezy PRIVATE sneezy::compiler_options)

# Copy binary to project root after build
add_custom_command(TARGET sneezy POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:sneezy> ${CMAKE_SOURCE_DIR}/sneezy
    COMMENT "Copying sneezy to project root"
)

# Apply sanitizers if enabled
if(ENABLE_SANITIZERS)
    target_link_libraries(sneezy PRIVATE sneezy::sanitizers)
endif()

# Preprocessor defines
target_compile_definitions(sneezy PRIVATE _DEFAULT_SOURCE)

if(ENABLE_NODUPLICATES)
    target_compile_definitions(sneezy PRIVATE NODUPLICATES)
endif()
if(ENABLE_HASH)
    target_compile_definitions(sneezy PRIVATE HASH)
endif()
if(ENABLE_NOTRACK)
    target_compile_definitions(sneezy PRIVATE NOTRACK)
endif()
if(ENABLE_LIMITED_ITEMS)
    target_compile_definitions(sneezy PRIVATE LIMITED_ITEMS)
endif()
if(ENABLE_DEBUG)
    target_compile_definitions(sneezy PRIVATE DEBUG)
endif()
if(ENABLE_SITELOCK)
    target_compile_definitions(sneezy PRIVATE SITELOCK)
endif()

# Format target - run clang-format on all source files
add_custom_target(format
    COMMAND clang-format -i ${CMAKE_SOURCE_DIR}/code/*.c ${CMAKE_SOURCE_DIR}/code/*.h
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running clang-format on source files"
)

# Symlink compile_commands.json to project root for IDE support
file(CREATE_LINK
    ${CMAKE_BINARY_DIR}/compile_commands.json
    ${CMAKE_SOURCE_DIR}/compile_commands.json
    SYMBOLIC
)
