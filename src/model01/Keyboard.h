// -*- c++ -*-

#pragma once

#include <Arduino.h>

//#include "Color.h"
#include "KeyswitchData.h"

// I think all we need is a forward declaration here
struct Color;

// This needs to be a macro so we can check the keymap definitions
#define TOTAL_KEYS 64

// Backward compatibility stuff here

#define HARDWARE_IMPLEMENTATION kaleidoscope::model01::Keyboard

#define COLS 16
#define ROWS 4

typedef Color cRGB;
typedef Color CRGB;

// End backcompat


namespace kaleidoscope {
namespace model01 {


class Keyboard {
 public:
  // This class should really be a singleton, but it probably costs a few bytes for the
  // extra getInstance() method that would be required to do that.
  Keyboard();

  static constexpr KeyAddr total_keys = TOTAL_KEYS;

  // Backcompat

  // Forward led functions
  void syncLeds() {
    return updateLeds();
  }
  void setCrgbAt(byte row, byte col, cRGB color) {
    return setLedColor(KeyAddr(row * 8 + col), color);
  }
  void setCrgbAt(uint8_t i, cRGB crgb) {
    return setLedColor(LedAddr(i), crgb);
  }
  cRGB getCrgbAt(uint8_t i) {
    return getLedColor(LedAddr(i));
  }

  // Just ignore key masking for now; it will be handled elsewhere
  void maskKey(byte row, byte col) {}
  void unMaskKey(byte row, byte col) {}
  bool isKeyMasked(byte row, byte col) { return false; }
  void maskHeldKeys(void) {}

  // void actOnMatrixScan(void); // private!

  // These are only used by TestMode & MagicCombo
  // keydata_t leftHandState;
  // keydata_t rightHandState;
  // keydata_t previousLeftHandState;
  // keydata_t previousRightHandState;

  // End backcompat


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

  static Scanner scanners_[2];

  union KeyboardState {
    KeyswitchData hands[2];
    byte banks[Keyboard::total_keys / 8];  // CHAR_BIT
  };
  KeyboardState keyboard_state_;
  KeyboardState prev_keyboard_state_;

  // I'm not sure we need this conversion function. On the other hand, maybe it should be
  // public...
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
