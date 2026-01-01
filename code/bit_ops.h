#pragma once

#include <type_traits>

#define IS_SET(flag, bit) ((flag) & (bit))

#define SET_BIT(var, bit) \
  ((var) = (std::remove_reference_t<decltype(var)>)((var) | (bit)))

#define REMOVE_BIT(var, bit) \
  ((var) = (std::remove_reference_t<decltype(var)>)((var) & ~(std::remove_reference_t<decltype(var)>)(bit)))

#define TOGGLE_BIT(var, bit) \
  ((var) = (std::remove_reference_t<decltype(var)>)((var) ^ (bit)))

#define SWITCH(a, b) \
  {                  \
    (a) ^= (b);      \
    (b) ^= (a);      \
    (a) ^= (b);      \
  }
