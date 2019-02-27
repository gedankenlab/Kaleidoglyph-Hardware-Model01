// -*- c++ -*-

#pragma once

#include <Arduino.h>

#include "model01/KeyAddr.h"


namespace kaleidoglyph {

class LedAddr {

 private:
  byte addr_;

  // Translate from KeyAddr to LedAddr by using this table. The index in the array is the
  // KeyAddr; the value is the LedAddr.
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

 public:
  LedAddr() = default;

  // The single-argument constructor is `explicit` to prevent automatic casting of
  // integers to LedAddr objects, and constexpr to allow the creation of compile-time
  // constant LedAddr objects.
  explicit constexpr
  LedAddr(byte addr) : addr_{addr} {}

  explicit
  LedAddr(KeyAddr k);

  // Read a LedAddr from an address in PROGMEM. This should be useful for sparse layers,
  // which will contain (LedAddr,Key) pairs.
  void readFromProgmem(const LedAddr& pgm_led_addr) {
    addr_ = pgm_read_byte(&pgm_led_addr.addr_);
  }

  
  // Comparison operators for use with other LedAddr objects
  bool operator==(const LedAddr& other) const {
    return this->addr_ == other.addr_;
  }
  bool operator!=(const LedAddr& other) const {
    return this->addr_ != other.addr_;
  }
  bool operator>(const LedAddr& other) const {
    return this->addr_ > other.addr_;
  }
  bool operator<(const LedAddr& other) const {
    return this->addr_ < other.addr_;
  }
  bool operator>=(const LedAddr& other) const {
    return this->addr_ >= other.addr_;
  }
  bool operator<=(const LedAddr& other) const {
    return this->addr_ <= other.addr_;
  }

  // Assignment & arithmetic operators (LedAddr)
  LedAddr &operator=(const LedAddr& other) {
    this->addr_ = other.addr_;
    return *this;
  }
  LedAddr &operator+=(const LedAddr& other) {
    this->addr_ += other.addr_;
    return *this;
  }
  LedAddr &operator-=(const LedAddr& other) {
    this->addr_ -= other.addr_;
    return *this;
  }

  // Increment & decrement unary operators
  LedAddr &operator++() { // prefix
    ++addr_;
    return *this;
  }
  LedAddr &operator--() { // prefix
    --addr_;
    return *this;
  }
  LedAddr operator++(int) { // postfix
    LedAddr tmp(addr_++);
    return tmp;
  }
  LedAddr operator--(int) { // postfix
    LedAddr tmp(addr_--);
    return tmp;
  }

  explicit
  operator byte() {
    return addr_;
  }
  // Maybe I should provide a cast operator to convert to LedAddr from KeyAddr?

};

inline LedAddr getProgmemLedAddr(const LedAddr& pgm_led_addr) {
  return LedAddr(pgm_read_byte(&pgm_led_addr));
}

} // namespace kaleidoglyph {
