// -*- c++ -*-

#pragma once

#include <Arduino.h>
#include "KeyAddr.h"

namespace kaleidoscope {
namespace model01 {

struct LedAddr {

  // Default addr value is invalid (or, at least, not a physical keyswitch)
  byte addr {0xFF};

  LedAddr() = default;

  // The single-argument constructor is `explicit` to prevent automatic casting of
  // integers to LedAddr objects, and constexpr to allow the creation of compile-time
  // constant LedAddr objects.
  explicit constexpr
  LedAddr(byte addr) : addr{addr} {}

  static constexpr PROGMEM byte key_led_map[] = {
    27, 26, 20, 19, 12, 11,  4,  3,
    28, 25, 21, 18, 13, 10,  5,  2,
    29, 24, 22, 17, 14,  9,  6,  1,
    30, 31, 23, 16, 15,  8,  7,  0,

    60, 59, 52, 51, 44, 43, 37, 36,
    61, 58, 53, 50, 45, 42, 38, 35,
    62, 57, 54, 49, 46, 41, 39, 34,
    63, 56, 55, 48, 47, 40, 32, 33,
  };

  explicit
  LedAddr(KeyAddr k) : addr{pgm_read_byte(&key_led_map[k.addr])} {}


  // Read a LedAddr from an address in PROGMEM. This should be useful for sparse layers,
  // which will contain (LedAddr,Key) pairs.
  void readFromProgmem(LedAddr const & pgm_key_addr) {
    addr = pgm_read_byte(&pgm_key_addr.addr);
  }

  
  // Comparison operators for use with other LedAddr objects
  bool operator==(LedAddr const & other) const {
    return this->addr == other.addr;
  }
  bool operator!=(LedAddr const & other) const {
    return this->addr != other.addr;
  }
  bool operator>(LedAddr const & other) const {
    return this->addr > other.addr;
  }
  bool operator<(LedAddr const & other) const {
    return this->addr < other.addr;
  }
  bool operator>=(LedAddr const & other) const {
    return this->addr >= other.addr;
  }
  bool operator<=(LedAddr const & other) const {
    return this->addr <= other.addr;
  }

  // Assignment & arithmetic operators (LedAddr)
  LedAddr & operator=(LedAddr const & other) {
    this->addr = other.addr;
    return *this;
  }
  LedAddr & operator+=(LedAddr const & other) {
    this->addr += other.addr;
    return *this;
  }
  LedAddr & operator-=(LedAddr const & other) {
    this->addr -= other.addr;
    return *this;
  }

  // Increment & decrement unary operators
  LedAddr & operator++() { // prefix
    ++addr;
    return *this;
  }
  LedAddr & operator--() { // prefix
    --addr;
    return *this;
  }
  LedAddr operator++(int) { // postfix
    LedAddr tmp(addr++);
    return tmp;
  }
  LedAddr operator--(int) { // postfix
    LedAddr tmp(addr--);
    return tmp;
  }

  // Maybe I should provide a cast operator to convert to LedAddr from KeyAddr?

};

} // namespace model01 {
} // namespace kaleidoscope {
