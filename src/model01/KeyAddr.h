// -*- c++ -*-

#pragma once

#include <Arduino.h>


namespace kaleidoscope {

namespace model01 {

// The Model01 has only 64 keys, and we only need one extra value to indicate an invalid
// key address, so that all fits in one byte.
typedef byte KeyAddr;
// The trouble with making KeyAddr just a typedef is that if we need to read one from
// PROGMEM or EEPROM, we can't do it without Kaleidoscope (or whatever plugin) knowing
// what the underlying type is: pgm_read_byte() vs pgm_read_word(). This means that it
// actually needs to be a class, which means all the operator overloading that I wanted to
// avoid. Oh, well...

struct KeyAddr {
 private:
  byte addr_;

 public:
  KeyAddr() { addr_ = TOTAL_KEYS; }
  constexpr KeyAddr(byte addr) : addr_(addr) {}

  // Comparison operators for use with basic integer types
  constexpr bool operator==(byte addr) {
    return addr_ == addr;
  }
  constexpr bool operator!=(byte addr) {
    return addr_ != addr;
  }
  constexpr bool operator>(byte addr) {
    return addr_ > addr;
  }
  constexpr bool operator<(byte addr) {
    return addr_ < addr;
  }
  constexpr bool operator>=(byte addr) {
    return addr_ >= addr;
  }
  constexpr bool operator<=(byte addr) {
    return addr_ <= addr;
  }

  // Comparison operators for use with other KeyAddr objects
  constexpr KeyAddr& operator=(const KeyAddr& other) {
    this->addr_ = other.addr_;
    return *this;
  }
  constexpr bool operator==(const KeyAddr& other) {
    return this->addr_ == other.addr_;
  }
  constexpr bool operator!=(const KeyAddr& other) {
    return this->addr_ != other.addr_;
  }
  constexpr bool operator>(const KeyAddr& other) {
    return this->addr_ > other.addr_;
  }
  constexpr bool operator<(const KeyAddr& other) {
    return this->addr_ < other.addr_;
  }
  constexpr bool operator>=(const KeyAddr& other) {
    return this->addr_ >= other.addr_;
  }
  constexpr bool operator<=(const KeyAddr& other) {
    return this->addr_ <= other.addr_;
  }

  // Other operators (integers only)
  constexpr KeyAddr& operator=(byte addr) {
    addr_ = addr;
    return *this;
  }
  constexpr bool operator+=(byte addr) {
    return addr_ += addr;
  }
  constexpr bool operator-=(byte addr) {
    return addr_ -= addr;
  }
  constexpr KeyAddr& operator++() {
    ++addr_;
    return *this;
  }
  constexpr KeyAddr operator++(int) {
    KeyAddr tmp(addr_++);
    return tmp;
  }
  constexpr KeyAddr& operator--() {
    --addr_;
    return *this;
  }
  constexpr KeyAddr operator--(int) {
    KeyAddr tmp(addr_--);
    return tmp;
  }

  // Any key address that's out of bounds is not valid. I'm not sure this is really
  // desirable, but I'm leaving this here for now.
  constexpr bool operator!() {
    return KeyAddr >= TOTAL_KEYS;
  }

  // The big thing that's missing here is some way to use a KeyAddr object as a subscript
  // operand. I'll have to experiment to find out if that can work.
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
