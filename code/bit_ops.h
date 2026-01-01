#pragma once

#include <concepts>

// Concept for types that support bitwise operations (integers and enums)
template <typename T>
concept BitfieldType = std::integral<T> || std::is_enum_v<T>;

template <BitfieldType T, BitfieldType U>
constexpr bool IS_SET(T flag, U bit) {
  return (flag & bit) != 0;
}

template <BitfieldType T, BitfieldType U>
constexpr void SET_BIT(T& var, U bit) {
  var = static_cast<T>(var | bit);
}

template <BitfieldType T, BitfieldType U>
constexpr void REMOVE_BIT(T& var, U bit) {
  var = static_cast<T>(var & ~bit);
}

template <BitfieldType T, BitfieldType U>
constexpr void TOGGLE_BIT(T& var, U bit) {
  var = static_cast<T>(var ^ bit);
}

template <typename T, typename U>
  requires std::assignable_from<T&, U> && std::assignable_from<U&, T>
constexpr void SWITCH(T& a, U& b) {
  T tmp = a;
  a = static_cast<T>(b);
  b = static_cast<U>(tmp);
}
