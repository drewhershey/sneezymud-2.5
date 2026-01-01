#pragma once

#include <stdio.h>
#include <stdlib.h>

#define CREATE(result, type, number)                  \
  do {                                                \
    (result) = (type*)calloc((number), sizeof(type)); \
    if (!(result)) {                                  \
      perror("malloc failure");                       \
      abort();                                        \
    }                                                 \
  } while (0)

#define RECREATE(result, type, number)                         \
  do {                                                         \
    void* _temp;                                               \
    _temp = realloc((void*)(result), sizeof(type) * (number)); \
    if (!(_temp)) {                                            \
      perror("realloc failure");                               \
      abort();                                                 \
    }                                                          \
    (result) = (type*)_temp;                                   \
  } while (0)

typedef struct {
    void* ptr;
    size_t count;
    size_t size;
} alloc_result;

[[nodiscard]] static inline alloc_result alloc_or_die(size_t count,
  size_t size) {
  void* ptr = calloc(count, size);
  if (!ptr) {
    perror("allocation failure");
    abort();
  }
  alloc_result result = {ptr, count, size};
  return result;
}

#define create(type, count) (type*)alloc_or_die(count, sizeof(type)).ptr
