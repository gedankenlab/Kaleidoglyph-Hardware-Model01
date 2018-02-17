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
    // event of an MCU that's big-endian, these two bytes would need to be swapped. This
    // struct should probably be dropped entirely; it's just here for historical reasons
    // at this point. The comment about endian-ness is still important, though.
    uint8_t ldata;  // low bits (keycode)
    uint8_t hdata;  // high bits (flags)
  };

  // For the following members, I would use a uint16_t for all the bitfields for clarity
  // (and I think it would make the endian-ness issue moot), but it would mean that every
  // time a single-byte or smaller bitfield component was accessed, we'd get two bytes
  // because it always uses the underlying type. That could be a big performance hit on an
  // 8-bit processor, so I'm just using a uint16_t where it's needed.

  // Keyboard key type: 8 bits for keycode, 4 modifier flags, one modifier hand flag, and
  // three bits for type identification (type is all zeros).
  struct {
    byte keycode;
    byte mods : 4, mods_right : 1, type : 3;
  } keyboard;

  // Consumer Control key type: 10 bits for keycode, 6 for type
  struct {
    uint16_t keycode : 10, type : 6;
  } consumer;

  // System Control key type: 8 bits for keycode, 8 for type
  struct {
    byte keycode;
    byte type;
  } system;

  // Mouse key type: 8 bits for keycode, 8 for type
  struct {
    byte keycode;
    byte type;
  } mouse;

  // Plugin key type: only two bits for identifying the type (could be just one), and the
  // rest can be used for identifying plugins, and storing values
  struct {
    uint16_t keycode : 14, type : 2;
  } plugin;

  // Constructors
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

  // These mod bits are in the same order as the mod bits in the HID keyboard report for
  // efficiency in passing them on. If they get rearranged, that will mess things up.
  static constexpr byte mod_control = B0001;  // (1 << 0)
  static constexpr byte mod_shift   = B0010;  // (1 << 1)
  static constexpr byte mod_alt     = B0100;  // (1 << 2)
  static constexpr byte mod_gui     = B1000;  // (1 << 3)

  // get a modifiers byte that can be passed to the keyboard HID report directly
  byte mods() {
    // If it's not a keyboard key, no mods are applied, so bail out. This test is probably
    // inefficient, because we should always be doing it before calling this function, but
    // it's safer this way.
    if (this->keyboard.type != Key::keyboard_type_id)
      return 0;

    byte modifiers(this->keyboard.mods);
    // If the fifth bit (mods_right) is set, that means we have to shift the mods four
    // bits to match the right-hand modifier bits in the HID report modifiers byte.
    if (this->keyboard.mods_right)
      modifiers <<= 4;
    return modifiers;
  }

  // Very commonly-used test functions. I've decided to use `Blank` instead of `NoKey`
  // because I think it's much clearer, especially in the light of my intention to
  // implement sparse layers.
  constexpr bool isTransparent() const {
    return (raw == 0xFFFF);
  }
  constexpr bool isBlank() const {
    return (raw == 0x0000);
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

  // Comparison operators (integers). I don't think these are really necessary, or even
  // desirable; we should remember to cast integers appropriately, instead. There's
  // probably a small benefit in performance, though.
  constexpr bool operator==(uint16_t val) const {
    return this->raw == val;
  }
  constexpr bool operator>=(uint16_t val) const {
    return this->raw >= val;
  }
  constexpr bool operator<=(uint16_t val) const {
    return this->raw <= val;
  }
  constexpr bool operator>(uint16_t val) const {
    return this->raw > val;
  }
  constexpr bool operator<(uint16_t val) const {
    return this->raw < val;
  }

  // Set key value from integer
  inline Key& operator=(uint16_t val) {
    this->raw = val;
    return *this;
  }
  // These two operators should be useful for decoding offsets
  inline Key& operator+=(uint16_t val) {
    this->raw += val;
    return *this;
  }
  inline Key& operator-=(uint16_t val) {
    this->raw -= val;
    return *this;
  }
  // Key object versions
  inline Key& operator+=(const Key& other) {
    this->raw += other.raw;
    return *this;
  }
  inline bool operator-=(const Key& other) {
    this->raw -= other.raw;
    return *this;
  }

  // Other arithmetic operators for use with offsets
  constexpr Key operator+(uint16_t val) const {
    return Key(this->raw + val);
  }
  constexpr Key operator-(uint16_t val) const {
    return Key(this->raw + val);
  }
  // Key object versions
  constexpr Key operator+(Key other) const {
    return Key(this->raw + other.raw);
  }
  constexpr Key operator-(Key other) const {
    return Key(this->raw - other.raw);
  }

  // These *_type_id constants are used to identify the type of the Key object
  static constexpr byte keyboard_type_id = B000;
  static constexpr byte consumer_type_id = B001000;
  static constexpr byte   system_type_id = B00100100;
  static constexpr byte    mouse_type_id = B00100101;
  static constexpr byte    layer_type_id = B00100110;
  static constexpr byte   plugin_type_id = B01;

  // Return the correct type (KeyClass) of the Key object. I expect KeyClass::plugin to be
  // more common than most of the core types, so I moved it up higher. These should be
  // sorted in descending order of frequency of use; I'm just guessing here.
  KeyClass type() {
    if (this->keyboard.type == Key::keyboard_type_id)
      return KeyClass::keyboard;
    if (this->plugin.type == Key::plugin_type_id)
      return KeyClass::plugin;
    if (this->layer.type == Key::layer_type_id)
      return KeyClass::layer;
    if (this->mouse.type == Key::mouse_type_id)
      return KeyClass::mouse;
    if (this->consumer.type == Key::consumer_type_id)
      return KeyClass::consumer;
    if (this->system.type == Key::system_type_id)
      return KeyClass::system;
    return KeyClass::unknown;
  }

};

} // namespace model01 {
} // namespace kaleidoscope {
