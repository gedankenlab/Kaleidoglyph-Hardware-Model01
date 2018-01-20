// -*- c++ -*-

#pragma once

#include <Arduino.h>

// instead of this:
#define HARDWARE_IMPLEMENTATION Model01
// try this:
namespace kaleidoscope::hardware = kaleidoscope::mode01;

#include "model01/Color.h"
#include "model01/KeyAddr.h"
#include "model01/LedAddr.h"
#include "model01/Scanner.h"

#define UNKNOWN_KEY_ADDR  kaleidoscope::model01::Keyboard::total_keys;
#define TOTAL_KEYS        kaleidoscope::model01::Keyboard::total_keys;


namespace kaleidoscope {

namespace model01 {

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

  union KeyboardState {
    struct {
      KeyData left_hand;
      KeyData right_hand;
    };
    byte rows[TOTAL_KEYS / 8];  // CHAR_BIT
  };
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
