# Sanitizer configuration for comprehensive runtime error detection
add_library(sanitizers INTERFACE)
add_library(sneezy::sanitizers ALIAS sanitizers)

# Comprehensive sanitizer configuration for production use
target_compile_options(sanitizers INTERFACE
    # AddressSanitizer - heap/stack/global buffer overflow detection
    -fsanitize=address
    -fsanitize-address-use-after-scope

    # UndefinedBehaviorSanitizer - comprehensive checks
    -fsanitize=undefined
    -fsanitize=float-divide-by-zero
    -fsanitize=integer
    -fsanitize=nullability

    # LeakSanitizer - memory leak detection
    -fsanitize=leak
)

target_link_options(sanitizers INTERFACE
    -fsanitize=address,undefined,leak
)
