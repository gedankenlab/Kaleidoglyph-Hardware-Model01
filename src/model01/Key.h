// -*- c++ -*-

#pragma once

#include <Arduino.h>
#include <stdint.h>

#define HID_KEYBOARD_LEFT_CONTROL	0xE0
#define HID_KEYBOARD_LEFT_SHIFT	0xE1
#define HID_KEYBOARD_LEFT_ALT	0xE2
#define HID_KEYBOARD_LEFT_GUI	0xE3
#define HID_KEYBOARD_RIGHT_CONTROL	0xE4
#define HID_KEYBOARD_RIGHT_SHIFT	0xE5
#define HID_KEYBOARD_RIGHT_ALT	0xE6
#define HID_KEYBOARD_RIGHT_GUI	0xE7

namespace kaleidoscope {
namespace model01 {

union Key {

  uint16_t raw;
  struct {
    // AVR is little-endian, which is why this file is here instead of in the core. In the
    // event of an MCU that's big-endian, these two bytes would need to be swapped
    uint8_t code;  // low bits (keycode)
    uint8_t meta;  // high bits (flags)
  };
  struct {
    uint8_t keycode;
    struct {
      uint8_t modifier_flags : 4, hand : 1, key_type : 3;
    };
  };

  Key() = default;
  constexpr Key(uint8_t _code, uint8_t _meta) : code(_code), meta(_meta) {}
  constexpr Key(uint16_t _raw) : raw(_raw) {}

  inline void readFromProgmem(const Key& progmem_key) {
    raw = pgm_read_word(&progmem_key);
  }
  inline void readFromEeprom(uint16_t eeprom_addr) {
    EEPROM.get(eeprom_addr, raw);
  }

  static constexpr Key createFromProgmem(const Key& progmem_key) {
    return Key(pgm_read_word(&progmem_key));
    /* This doesn't work in C++11, but it will in C++14
    Key new_key;
    new_key.readFromProgmem(progmem_key);
    return new_key;
    */
  }
  static inline Key createFromEeprom(uint16_t eeprom_addr) {
    // This can't be constexpr in C++11, but it can be in C++14
    Key new_key;
    EEPROM.get(eeprom_addr, new_key);
    return new_key;
  }

  constexpr bool isTransparent() const {
    return (raw == 0xFFFF);
  }
  constexpr bool isBlank() const {
    return (raw == 0x0000);
  }

  // Comparison operators (integers). I don't think these are really necessary, or even
  // desirable; we should remember to cast integers appropriately, instead. There's
  // probably a small benefit in performance, though.
  constexpr bool operator==(uint16_t raw) const {
    return this->raw == raw;
  }
  constexpr bool operator>=(uint16_t raw) const {
    return this->raw >= raw;
  }
  constexpr bool operator<=(uint16_t raw) const {
    return this->raw <= raw;
  }
  constexpr bool operator>(uint16_t raw) const {
    return this->raw > raw;
  }
  constexpr bool operator<(uint16_t raw) const {
    return this->raw < raw;
  }

  // Comparison operators (Key objects)
  constexpr bool operator==(const Key& other) const {
    return this->raw == other.raw;
  }
  constexpr bool operator!=(const Key& other) const {
    return this->raw != other.raw;
  }
  constexpr bool operator>(const Key& other) const {
    return this->raw > other.raw;
  }
  constexpr bool operator<(const Key& other) const {
    return this->raw < other.raw;
  }
  constexpr bool operator>=(const Key& other) const {
    return this->raw >= other.raw;
  }
  constexpr bool operator<=(const Key& other) const {
    return this->raw <= other.raw;
  }

  // Set key value from integer
  inline Key& operator=(uint16_t raw) {
    this->raw = raw;
    return *this;
  }
  // These two operators should be useful for decoding offsets
  inline Key& operator+=(uint16_t raw) {
    this->raw += raw;
    return *this;
  }
  inline Key& operator-=(uint16_t raw) {
    this->raw -= raw;
    return *this;
  }
  inline Key& operator+=(const Key& other) {
    this->raw += other.raw;
    return *this;
  }
  inline bool operator-=(const Key& other) {
    this->raw -= other.raw;
    return *this;
  }

  // Other arithmetic operators for use with offsets
  constexpr Key operator+(Key other) const {
    return Key(this->raw + other.raw);
  }
  constexpr Key operator-(Key other) const {
    return Key(this->raw - other.raw);
  }

};

} // namespace model01 {
} // namespace kaleidoscope {
