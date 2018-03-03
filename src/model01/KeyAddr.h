// -*- c++ -*-

#pragma once

#include <Arduino.h>


namespace kaleidoscope {

// The Model01 has only 64 keys, and we only need one extra value to indicate an invalid
// key address, so that all fits in one byte.
// typedef byte KeyAddr;

// I mistakenly thought that there was a problem with reading from PROGMEM and/or EEPROM
// that would be solved by using a struct instead of a basic typedef, so I implemented all
// this operator overloading. Now that I've done it, I'm going to try it out anyway, even
// though it doesn't solve the problem I thought it would.
struct KeyAddr {

  // Default addr value is invalid (or, at least, not a physical keyswitch)
  byte addr {0xFF};

  KeyAddr() = default;

  // The single-argument constructor is `explicit` to prevent automatic casting of
  // integers to KeyAddr objects, and constexpr to allow the creation of compile-time
  // constant KeyAddr objects.
  explicit constexpr
  KeyAddr(byte addr) : addr{addr} {}

  // This (row,col) constructor was conceived as a transitional helper, but should
  // probably just be removed, since the "row" and "col" values don't match the old ones.
  constexpr
  KeyAddr(byte row, byte col) : addr((row * 8) + col) {}

  // Read a KeyAddr from an address in PROGMEM. This should be useful for sparse layers,
  // which will contain (KeyAddr,Key) pairs.
  void readFromProgmem(KeyAddr const &pgm_key_addr) {
    addr = pgm_read_byte(&pgm_key_addr.addr);
  }

  
  // Comparison operators for use with other KeyAddr objects
  bool operator==(KeyAddr const &other) const {
    return this->addr == other.addr;
  }
  bool operator!=(KeyAddr const &other) const {
    return this->addr != other.addr;
  }
  bool operator>(KeyAddr const &other) const {
    return this->addr > other.addr;
  }
  bool operator<(KeyAddr const &other) const {
    return this->addr < other.addr;
  }
  bool operator>=(KeyAddr const &other) const {
    return this->addr >= other.addr;
  }
  bool operator<=(KeyAddr const &other) const {
    return this->addr <= other.addr;
  }

  // Assignment & arithmetic operators (KeyAddr)
  KeyAddr &operator=(KeyAddr const &other) {
    this->addr = other.addr;
    return *this;
  }
  KeyAddr &operator+=(KeyAddr const &other) {
    this->addr += other.addr;
    return *this;
  }
  KeyAddr &operator-=(KeyAddr const &other) {
    this->addr -= other.addr;
    return *this;
  }

  // Increment & decrement unary operators
  KeyAddr &operator++() { // prefix
    ++addr;
    return *this;
  }
  KeyAddr &operator--() { // prefix
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

} // namespace kaleidoscope {
