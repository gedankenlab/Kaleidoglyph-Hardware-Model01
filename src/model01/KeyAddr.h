// -*- c++ -*-

#pragma once

#include <Arduino.h>

// Can this class be used as an iterator for range-based for loops? Layer classes would
// need appropriate begin() & end() methods. See:
// https://www.cprogramming.com/c++11/c++11-ranged-for-loop.html

// I think it won't work; a separate iterator is needed because we'd need to store a
// pointer, which would triple the storage size of a KeyAddr.

namespace kaleidoscope {
namespace model01 {

// The Model01 has only 64 keys, and we only need one extra value to indicate an invalid
// key address, so that all fits in one byte.
// typedef byte KeyAddr;

// I mistakenly thought that there was a problem with reading from PROGMEM and/or EEPROM
// that would be solved by using a struct instead of a basic typedef, so I implemented all
// this operator overloading. Now that I've done it, I'm going to try it out anyway, even
// though it doesn't solve the problem I thought it would.
struct KeyAddr {

  byte addr {0xFF};

  // Default constructor initializes with an invalid addr
  KeyAddr() = default;

  explicit constexpr KeyAddr(byte addr) : addr{addr} {}

  // I feel like `((row << 3) | col)` should be faster
  constexpr KeyAddr(byte row, byte col) : addr((row * 8) + col) {}

#if 0
  void readFromProgmem(const KeyAddr& pgm_key_addr) {
    addr = pgm_read_byte(&pgm_key_addr.addr);
  }
  static KeyAddr createFromProgmem(const KeyAddr& pgm_key_addr) {
    KeyAddr k;
    k.addr = pgm_read_byte(&pgm_key_addr.addr);
    return k;
  }
#endif

  // Comparison operators for use with other KeyAddr objects
  constexpr bool operator==(const KeyAddr& other) const {
    return this->addr == other.addr;
  }
  constexpr bool operator!=(const KeyAddr& other) const {
    return this->addr != other.addr;
  }
  constexpr bool operator>(const KeyAddr& other) const {
    return this->addr > other.addr;
  }
  constexpr bool operator<(const KeyAddr& other) const {
    return this->addr < other.addr;
  }
  constexpr bool operator>=(const KeyAddr& other) const {
    return this->addr >= other.addr;
  }
  constexpr bool operator<=(const KeyAddr& other) const {
    return this->addr <= other.addr;
  }

  // Note: we can't use `constexpr` with these ones with C++11, because that implies
  // `const`, which we don't want. So, we use `inline` instead, which amounts to the same
  // thing. In C++14, these can become `constexpr`, but without the `const`.

  // Assignment & arithmetic operators (KeyAddr)
  KeyAddr& operator=(const KeyAddr& other) {
    this->addr = other.addr;
    return *this;
  }
  KeyAddr& operator+=(const KeyAddr& other) {
    this->addr += other.addr;
    return *this;
  }
  KeyAddr& operator-=(const KeyAddr& other) {
    this->addr -= other.addr;
    return *this;
  }

  // Increment & decrement unary operators
  KeyAddr& operator++() { // prefix
    ++addr;
    return *this;
  }
  KeyAddr& operator--() { // prefix
    --addr;
    return *this;
  }
  KeyAddr operator++(int) { // postfix
    KeyAddr tmp(addr++);
    return tmp;
  }
  KeyAddr operator--(int) { // postfix
    KeyAddr tmp(addr--);
    return tmp;
  }

  // Maybe I should provide a cast operator to convert to LedAddr from KeyAddr?

};

} // namespace model01 {
} // namespace kaleidoscope {
