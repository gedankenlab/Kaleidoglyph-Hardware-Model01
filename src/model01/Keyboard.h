// -*- c++ -*-

#pragma once

#include <Arduino.h>

// instead of this:
#define HARDWARE_IMPLEMENTATION Model01
// try this:
namespace kaleidoscope::hardware = kaleidoscope::mode01;

#include "model01/Scanner.h"

#define UNKNOWN_KEY_ADDR  kaleidoscope::model01::Keyboard::total_keys;
#define TOTAL_KEYS        kaleidoscope::model01::Keyboard::total_keys;


namespace kaleidoscope {

namespace model01 {

// The Model01 has only 64 keys, and we only need one extra value to indicate an invalid
// key address, so that all fits in one byte.
typedef byte KeyAddr;

// I think we should also have a type for LED index. Warning: this is tricky, because
// "KeyAddr" & "LedAddr" are actually the same type, so we can't use two functions, one of
// which takes a KeyAddr, and a different one that takes LedAddr as a param. The compiler
// will catch it, though.
typedef byte LedAddr;

// In the old cRGB struct, the order of the bytes (b,g,r) was important because brace
// initialization was used. for the Color struct, I'm using a real constructor, so that
// won't matter, but I'll leave the order as it was for brace initialization. What really
// matters is the order of the arguments when calling the twi function that sends the
// commands to the LED controller module, and that's handled by the functions that make
// those calls, not this struct.
struct Color {
  byte b;
  byte g;
  byte r;

  Color() = default; // Does not initialize to zeros, unless `Color c = {};`
  // I'm not really sure why constexpr would help here. I should experiment to see if it
  // produces smaller code. I'm also not sure about the constructor above; this is obtuse
  // C++ magic.
  constexpr Color(byte red, byte green, byte blue)
    : r(red), g(green), b(blue) {}
};


union KeyboardState {
  struct {
    KeyData left_hand;
    KeyData right_hand;
  };
  byte rows[TOTAL_KEYS / 8];  // ROWS no good
};


class Keyboard {
 public:
  // This class should really be a singleton, but it probably costs a few bytes for the
  // extra getInstance() method that would be required to do that.
  Model01();

  static constexpr KeyAddr total_keys = 64;

  // New API
  void scanMatrix();
  // should probably return KeyswitchEvent instead
  KeyAddr nextKeyswitchEvent(KeyAddr key_addr);
  // I really don't think we need this function, but maybe it will be useful
  byte getKeyswitchState(KeyAddr key_addr);

  // Update all LEDs to values set by set*Color() functions below
  void updateLeds();

  // These functions operate on LedAddr values, which are different from corresponding KeyAddr values
  Color getLedColor(LedAddr led_addr);
  void  setLedColor(LedAddr led_addr, Color color);

  // These are the KeyAddr versions, which call the LedAddr functions
  Color getKeyColor(KeyAddr key_addr);
  void  setKeyColor(KeyAddr key_addr, Color color);

  // I'm leaving this functions alone for now; they shall remain mysterious
  void setup();

  // This function is used by TestMode
  void setKeyscanInterval(uint8_t interval);

 private:
  static constexpr byte HAND_BIT = B00100000;

  // The scanner and KeyData types shouldn't be part of the interface, but if they're
  // members, there's no way to hide those types from anything that needs to access class
  // model01::Keyboard. I could define them in Model01.cpp instead, as static variables,
  // but that doesn't seem like a great approach, either.
  static Scanner scanners_[2];
  // I'm still not sure about these needing to be static -- it's because we really don't
  // want multiple instances of Keyboard, especially not multiple instances of the same
  // Scanner/Controller object
  //static KeyboardioScanner scanner_l_;
  //static KeyboardioScanner scanner_r_;

  KeyboardState keyboard_state_;
  KeyboardState prev_keyboard_state_;

  // I'm not sure we need this conversion function
  LedAddr getLedAddr(KeyAddr key_addr);

  // special functions for Model01; make private if possible
  void enableHighPowerLeds();
  void enableScannerPower();
  boolean ledPowerFault();

  // This doesn't seem to be called anywhere
  void rebootBootloader();
};

} // namespace model01 {

} // namespace kaleidoscope {


#define KEYMAP_STACKED(                                                 \
               r0c0, r0c1, r0c2, r0c3, r0c4, r0c5, r0c6,                \
               r1c0, r1c1, r1c2, r1c3, r1c4, r1c5, r1c6,                \
               r2c0, r2c1, r2c2, r2c3, r2c4, r2c5,                      \
               r3c0, r3c1, r3c2, r3c3, r3c4, r3c5, r2c6,                \
               r0c7, r1c7, r2c7, r3c7,                                  \
               r3c6,                                                    \
                                                                        \
               r0c9,  r0c10, r0c11, r0c12, r0c13, r0c14, r0c15,         \
               r1c9,  r1c10, r1c11, r1c12, r1c13, r1c14, r1c15,         \
                      r2c10, r2c11, r2c12, r2c13, r2c14, r2c15,         \
               r2c9,  r3c10, r3c11, r3c12, r3c13, r3c14, r3c15,         \
               r3c8,  r2c8,  r1c8, r0c8,                                \
               r3c9)                                                    \
  {                                                                     \
    {r0c0, r0c1, r0c2, r0c3, r0c4, r0c5, r0c6, r0c7, r0c8, r0c9, r0c10, r0c11, r0c12, r0c13, r0c14, r0c15}, \
    {r1c0, r1c1, r1c2, r1c3, r1c4, r1c5, r1c6, r1c7, r1c8, r1c9, r1c10, r1c11, r1c12, r1c13, r1c14, r1c15}, \
    {r2c0, r2c1, r2c2, r2c3, r2c4, r2c5, r2c6, r2c7, r2c8, r2c9, r2c10, r2c11, r2c12, r2c13, r2c14, r2c15}, \
    {r3c0, r3c1, r3c2, r3c3, r3c4, r3c5, r3c6, r3c7, r3c8, r3c9, r3c10, r3c11, r3c12, r3c13, r3c14, r3c15}, \
  }

#define KEYMAP(                                                                                     \
  r0c0, r0c1, r0c2, r0c3, r0c4, r0c5, r0c6,        r0c9,  r0c10, r0c11, r0c12, r0c13, r0c14, r0c15, \
  r1c0, r1c1, r1c2, r1c3, r1c4, r1c5, r1c6,        r1c9,  r1c10, r1c11, r1c12, r1c13, r1c14, r1c15, \
  r2c0, r2c1, r2c2, r2c3, r2c4, r2c5,                     r2c10, r2c11, r2c12, r2c13, r2c14, r2c15, \
  r3c0, r3c1, r3c2, r3c3, r3c4, r3c5, r2c6,        r2c9,  r3c10, r3c11, r3c12, r3c13, r3c14, r3c15, \
              r0c7, r1c7, r2c7, r3c7,                             r3c8,  r2c8,  r1c8, r0c8,         \
                          r3c6,                                          r3c9)                      \
  {                                                                                                 \
    {r0c0, r0c1, r0c2, r0c3, r0c4, r0c5, r0c6, r0c7, r0c8, r0c9, r0c10, r0c11, r0c12, r0c13, r0c14, r0c15}, \
    {r1c0, r1c1, r1c2, r1c3, r1c4, r1c5, r1c6, r1c7, r1c8, r1c9, r1c10, r1c11, r1c12, r1c13, r1c14, r1c15}, \
    {r2c0, r2c1, r2c2, r2c3, r2c4, r2c5, r2c6, r2c7, r2c8, r2c9, r2c10, r2c11, r2c12, r2c13, r2c14, r2c15}, \
    {r3c0, r3c1, r3c2, r3c3, r3c4, r3c5, r3c6, r3c7, r3c8, r3c9, r3c10, r3c11, r3c12, r3c13, r3c14, r3c15}, \
  }
