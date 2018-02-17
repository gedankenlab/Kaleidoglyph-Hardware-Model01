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
 private:
  byte addr_;

 public:
  // Default constructor initializes with an invalid addr
  KeyAddr() : addr_(Keyboard::total_keys) {}

  constexpr KeyAddr(byte addr) : addr_(addr) {}

  // I feel like `((row << 3) | col)` should be faster
  constexpr KeyAddr(byte row, byte col) : addr_((row * 8) + col) {}

  // This avoids copying, so maybe it's more efficient than using the cast operator
  inline byte& addr() {
    return addr;
  }

  inline void readFromProgmem(const KeyAddr& pgm_key_addr) {
    addr_ = pgm_read_byte(&pgm_key_addr.addr_);
  }
  static inline KeyAddr createFromProgmem(const KeyAddr& pgm_key_addr) {
    KeyAddr key_addr;
    key_addr.addr_ = pgm_read_byte(&pgm_key_addr.addr_);
    return key_addr;
  }

  // Comparison operators for use with other KeyAddr objects
  constexpr bool operator==(const KeyAddr& other) const {
    return this->addr_ == other.addr_;
  }
  constexpr bool operator!=(const KeyAddr& other) const {
    return this->addr_ != other.addr_;
  }
  constexpr bool operator>(const KeyAddr& other) const {
    return this->addr_ > other.addr_;
  }
  constexpr bool operator<(const KeyAddr& other) const {
    return this->addr_ < other.addr_;
  }
  constexpr bool operator>=(const KeyAddr& other) const {
    return this->addr_ >= other.addr_;
  }
  constexpr bool operator<=(const KeyAddr& other) const {
    return this->addr_ <= other.addr_;
  }

  // Comparison operators for use with basic integer types
  constexpr bool operator==(byte addr) const {
    return addr_ == addr;
  }
  constexpr bool operator!=(byte addr) const {
    return addr_ != addr;
  }
  constexpr bool operator>(byte addr) const {
    return addr_ > addr;
  }
  constexpr bool operator<(byte addr) const {
    return addr_ < addr;
  }
  constexpr bool operator>=(byte addr) const {
    return addr_ >= addr;
  }
  constexpr bool operator<=(byte addr) const {
    return addr_ <= addr;
  }

  // Note: we can't use `constexpr` with these ones with C++11, because that implies
  // `const`, which we don't want. So, we use `inline` instead, which amounts to the same
  // thing. In C++14, these can become `constexpr`, but without the `const`.

  // Assignment & arithmetic operators (KeyAddr)
  inline KeyAddr& operator=(const KeyAddr& other) {
    this->addr_ = other.addr_;
    return *this;
  }
  inline KeyAddr& operator+=(const KeyAddr& other) {
    this->addr_ += other.addr_;
    return *this;
  }
  inline KeyAddr& operator-=(const KeyAddr& other) {
    this->addr_ -= other.addr_;
    return *this;
  }

  // Assignment & arithmetic operators (integers)
  inline KeyAddr& operator=(byte addr) {
    addr_ = addr;
    return *this;
  }
  inline KeyAddr& operator+=(byte addr) {
    addr_ += addr;
    return *this;
  }
  inline KeyAddr& operator-=(byte addr) {
    addr_ -= addr;
    return *this;
  }

  // Increment & decrement unary operators
  inline KeyAddr& operator++() { // prefix
    ++addr_;
    return *this;
  }
  inline KeyAddr& operator--() { // prefix
    --addr_;
    return *this;
  }
  inline KeyAddr operator++(int) { // postfix
    KeyAddr tmp(addr_++);
    return tmp;
  }
  inline KeyAddr operator--(int) { // postfix
    KeyAddr tmp(addr_--);
    return tmp;
  }

  // Any key address that's out of bounds is not valid. I'm not sure this is really
  // desirable, but I'm leaving this here for now.
  constexpr bool operator!() const {
    return KeyAddr >= Keyboard::total_keys;
  }

  // The big thing that's missing here is some way to use a KeyAddr object as a subscript
  // operand. I'll have to experiment to find out if that can work. Using the cast
  // operator does the trick, but maybe it's better to overload the subscript operator of
  // classes that should be using it as a subscript.
  operator byte() {
    return addr_;
  }

};

} // namespace model01 {


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

} // namespace kaleidoscope {
