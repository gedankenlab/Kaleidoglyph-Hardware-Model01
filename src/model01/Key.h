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
    uint8_t ldata;  // low bits (keycode)
    uint8_t hdata;  // high bits (flags)
  };

  // Keyboard key type: 8 bits for keycode, 4 modifier flags, one modifier hand flag, and
  // three bits for type identification (type is all zeros)
  struct {
    byte keycode;
    byte mods : 4, hand : 1, type : 3;
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

  static constexpr byte mod_control = B0001;
  static constexpr byte mod_shift   = B0010;
  static constexpr byte mod_alt     = B0100;
  static constexpr byte mod_gui     = B1000;

  static constexpr byte mods_left  = B0;
  static constexpr byte mods_right = B1;

  // get a modifiers byte that can be passed to the key report directly
  byte mods() {
    if (keyboard.type != Key::keyboard_type)
      return 0;
    byte modifiers(keyboard.mods);
    if (keyboard.hand == Key::mods_right)
      modifiers <<= 4;
    return modifiers;
  }

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

  static constexpr byte keyboard_type = B000;
  static constexpr byte consumer_type = B001000;
  static constexpr byte   system_type = B00100100;
  static constexpr byte    mouse_type = B00100101;
  static constexpr byte    layer_type = B00100110;
  static constexpr byte   plugin_type = B01;

  // returns the correct type of the Key object
  KeyClass type() {
    if (keyboard.type == Key::keyboard_type)
      return KeyClass::keyboard;
    if (consumer.type == Key::consumer_type)
      return KeyClass::consumer;
    if (system.type == Key::system_type)
      return KeyClass::system;
    if (mouse.type == Key::mouse_type)
      return KeyClass::mouse;
    if (layer.type == Key::layer_type)
      return KeyClass::layer;
    if (plugin.type == Key::plugin_type)
      return KeyClass::plugin;
    return KeyClass::unknown;
  }

};

enum class KeyClass : byte {
  keyboard,
  consumer,
  system,
  mouse,
  layer,
  plugin,
  unknown
};

} // namespace model01 {
} // namespace kaleidoscope {
