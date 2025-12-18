# Clang toolchain configuration
set(CMAKE_C_COMPILER clang)

# Use lld linker for faster linking
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld")

# Use llvm tools for consistency
find_program(LLVM_AR NAMES llvm-ar llvm-ar-18 llvm-ar-17)
find_program(LLVM_RANLIB NAMES llvm-ranlib llvm-ranlib-18 llvm-ranlib-17)
if(LLVM_AR)
    set(CMAKE_AR ${LLVM_AR})
endif()
if(LLVM_RANLIB)
    set(CMAKE_RANLIB ${LLVM_RANLIB})
endif()
