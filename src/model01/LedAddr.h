// -*- c++ -*-

#pragma once

#include <Arduino.h>

#include "model01/KeyAddr.h"


namespace kaleidoglyph {

// I need to decide if this constant should be defined here or in Keyboard.h
constexpr byte total_leds = 64;
#define TOTAL_LEDS_STR "64"


class LedAddr {

 private:
  byte addr_;

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
