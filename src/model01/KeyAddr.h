// -*- c++ -*-

#pragma once

#include <Arduino.h>
#include <stdint.h>


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

 private:
  // I'm really not sure it's worth the bother to keep this private
  byte addr_;

 public:

  KeyAddr() = default;

  // Without the `explicit` keyword, this constructor acts as the counterpart to the cast
  // operator overload: if a function `f(KeyAddr k)` gets passed an integer: `f(12)`, the
  // integer will be automatically passed to this constructor, creating a temporary
  // KeyAddr object.
  constexpr explicit KeyAddr(byte addr) : addr_{addr} {}

  // I feel like `((row << 3) | col)` should be faster. This is only for backwards
  // compatibility
  constexpr KeyAddr(byte row, byte col) : addr_((row * 8) + col) {}

  // This avoids copying, so maybe it's more efficient than using the cast operator
  byte& addr() {
    return addr_;
  }

#if 0
  void readFromProgmem(KeyAddr const & pgm_key_addr) {
    addr_ = pgm_read_byte(&pgm_key_addr.addr_);
  }
  static KeyAddr createFromProgmem(KeyAddr const & pgm_key_addr) {
    KeyAddr key_addr;
    key_addr.addr_ = pgm_read_byte(&pgm_key_addr.addr_);
    return key_addr;
  }
#endif

  // Comparison operators for use with other KeyAddr objects
  constexpr bool operator==(KeyAddr const & other) const {
    return this->addr_ == other.addr_;
  }
  constexpr bool operator!=(KeyAddr const & other) const {
    return this->addr_ != other.addr_;
  }
  constexpr bool operator>(KeyAddr const & other) const {
    return this->addr_ > other.addr_;
  }
  constexpr bool operator<(KeyAddr const & other) const {
    return this->addr_ < other.addr_;
  }
  constexpr bool operator>=(KeyAddr const & other) const {
    return this->addr_ >= other.addr_;
  }
  constexpr bool operator<=(KeyAddr const & other) const {
    return this->addr_ <= other.addr_;
  }

  // Note: we can't use `constexpr` with these ones with C++11, because that implies
  // `const`, which we don't want. So, we use `inline` instead, which amounts to the same
  // thing. In C++14, these can become `constexpr`, but without the `const`.

  // Assignment & arithmetic operators (KeyAddr)
  KeyAddr& operator=(KeyAddr const & other) {
    this->addr_ = other.addr_;
    return *this;
  }
  KeyAddr& operator+=(KeyAddr const & other) {
    this->addr_ += other.addr_;
    return *this;
  }
  KeyAddr& operator-=(KeyAddr const & other) {
    this->addr_ -= other.addr_;
    return *this;
  }

  // Increment & decrement unary operators
  KeyAddr& operator++() { // prefix
    ++addr_;
    return *this;
  }
  KeyAddr& operator--() { // prefix
    --addr_;
    return *this;
  }
  KeyAddr operator++(int) { // postfix
    KeyAddr tmp(addr_++);
    return tmp;
  }
  KeyAddr operator--(int) { // postfix
    KeyAddr tmp(addr_--);
    return tmp;
  }

};

#if 0
namespace keyaddr {

// Everything past this point should be unnecessary, but could possibly be helpful for
// plugins that want to affect whole rows or columns. Note that this code defines 8 rows &
// 8 columns, which is probably not what a user wants on a Model01 (the last four rows are
// on the right half of the keyboard.

// ROWS = 8
// COLS = 8
constexpr KeyAddr ROW_BITS = B00111000;
constexpr KeyAddr COL_BITS = B00000111;

// Conversion functions to help with back-compat and user addressing. I actually think
// that these addresses are not great, since the thumb arc should be a separate row, and
// the palm keys yet another. User addressing should maybe be this 3-tuple: {hand, row,
// col}, but that could be handled with #defines, or the build scripts.
constexpr byte row(KeyAddr key_addr) {
  return (key_addr & ROW_BITS) >> 3;
}

constexpr byte col(KeyAddr key_addr) {
  return (key_addr & COL_BITS);
}

constexpr KeyAddr addr(byte row, byte col) {
  return ((row << 3) | col);
}

} // namespace keyaddr {
#endif

} // namespace model01 {
} // namespace kaleidoscope {
