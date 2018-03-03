// -*- c++ -*-

#pragma once

#include <Arduino.h>

// Backward compatibility stuff here

#define HARDWARE_IMPLEMENTATION kaleidoscope::hardware::Keyboard

#define COLS 16
#define ROWS 4
#define TOTAL_KEYS 64

// End backcompat
#include "model01/KeyswitchData.h"

// I think all we need is a forward declaration here
#include "model01/Color.h"
#include "model01/LedAddr.h"
#include "model01/KeyAddr.h"
#include "model01/Scanner.h"

namespace kaleidoscope {
namespace hardware {



// backcompat
typedef Color cRGB;
typedef Color CRGB;

// This needs to be a macro so we can check the keymap definitions
#define TOTAL_KEYS_STR "64"
constexpr byte total_keys = 64;







class Keyboard {
 public:
  // This class should really be a singleton, but it probably costs a few bytes for the
  // extra getInstance() method that would be required to do that.
  Keyboard();

  // Backcompat

  // Forward led functions
  void syncLeds() {
    return updateLeds();
  }
  void setCrgbAt(KeyAddr k, cRGB color) {
    return setKeyColor(k, color);
  }
  void setCrgbAt(uint8_t i, cRGB crgb) {
    return setLedColor(LedAddr(i), crgb);
  }
  cRGB getCrgbAt(uint8_t i) {
    return getLedColor(LedAddr(i));
  }

  // Just ignore key masking for now; it will be handled elsewhere
  void maskKey(KeyAddr k) {}
  void unMaskKey(KeyAddr k) {}
  bool isKeyMasked(KeyAddr k) { return false; }
  void maskHeldKeys(void) {}

  void actOnMatrixScan(void); // private!

  // These are only used by TestMode & MagicCombo
  // keydata_t leftHandState;
  // keydata_t rightHandState;
  // keydata_t previousLeftHandState;
  // keydata_t previousRightHandState;

  // End backcompat


  // New API
  void scanMatrix();

  // should probably return KeyswitchEvent instead
  byte nextKeyswitchEvent(KeyAddr& k);
  // I really don't think we need this function, but maybe it will be useful
  byte keyswitchState(KeyAddr k) const;

  // Update all LEDs to values set by set*Color() functions below
  void updateLeds();

  // These functions operate on LedAddr values, which are different from corresponding KeyAddr values
  Color const &getLedColor(LedAddr led) const;
  void setLedColor(LedAddr led, Color color);

  // These are the KeyAddr versions, which call the LedAddr functions
  Color const &getKeyColor(KeyAddr k) const;
  void setKeyColor(KeyAddr k, Color color);

  // I'm leaving these functions alone for now; they shall remain mysterious
  void setup();

  // This function is used by TestMode
  void setKeyscanInterval(byte interval);

 private:
  static constexpr byte HAND_BIT = B00100000;

  Scanner scanners_[2];

  union KeyboardState {
    KeyswitchData hands[2];
    byte banks[total_keys / 8];  // CHAR_BIT
  };
  KeyboardState keyboard_state_;
  KeyboardState prev_keyboard_state_;

  // I'm not sure we need this conversion function. On the other hand, maybe it should be
  // public...
  LedAddr getLedAddr(KeyAddr key_addr) const;

  // special functions for Model01; make private if possible
  void enableHighPowerLeds();
  void enableScannerPower();
  boolean ledPowerFault();

  // This doesn't seem to be called anywhere
  void rebootBootloader();
};

} // namespace model01 {
} // namespace kaleidoscope {
