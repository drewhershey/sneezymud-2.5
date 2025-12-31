#pragma once

#define IS_SET(flag, bit) ((flag) & (bit))

#define SET_BIT(var, bit) ((var) = (typeof(var))((var) | (bit)))

#define REMOVE_BIT(var, bit) \
  ((var) = (typeof(var))((var) & ~(typeof(var))(bit)))

#define TOGGLE_BIT(var, bit) ((var) = (typeof(var))((var) ^ (bit)))

#define SWITCH(a, b) \
  {                  \
    (a) ^= (b);      \
    (b) ^= (a);      \
    (a) ^= (b);      \
  }
