// -*- c++ -*-

#pragma once

#include <Arduino.h>

// Backward compatibility stuff here
#define HARDWARE_IMPLEMENTATION kaleidoscope::hardware::Keyboard
// End backcompat

#include "model01/KeyswitchData.h"
#include "model01/Color.h"
#include "model01/LedAddr.h"
#include "model01/KeyAddr.h"
#include "model01/Scanner.h"

#include <kaleidoscope/KeyswitchState.h>
#include <kaleidoscope/KeyswitchEvent.h>
#include <kaleidoscope/cKeyAddr.h>

namespace kaleidoscope {
namespace hardware {

// This needs to be a macro so we can check the keymap definitions
#define TOTAL_KEYS_STR "64"
constexpr byte total_keys = 64;


class Keyboard {

 public:
  // This class should really be a singleton, but it probably costs a few bytes for the
  // extra getInstance() method that would be required to do that.
  Keyboard();

  // New API
  void scanMatrix();

  // should probably return KeyswitchEvent instead
  KeyswitchEvent nextKeyswitchEvent(KeyAddr& k);
  // I really don't think we need this function, but maybe it will be useful
  KeyswitchState keyswitchState(KeyAddr k) const;

  // Update all LEDs to values set by set*Color() functions below
  void updateLeds();

  // These functions operate on LedAddr values, which are different from corresponding KeyAddr values
  Color getLedColor(LedAddr led) const;
  void  setLedColor(LedAddr led, Color color);

  // These are the KeyAddr versions, which call the LedAddr functions
  Color getKeyColor(KeyAddr k) const;
  void  setKeyColor(KeyAddr k, Color color);

  // I'm leaving these functions alone for now; they shall remain mysterious
  void setup();

  // This function is used by TestMode
  void setKeyscanInterval(byte interval);

 private:
  static constexpr byte HAND_BIT = B00100000;

  Scanner scanners_[2];

  union KeyswitchScan {
    KeyswitchData hands[2];
    byte banks[total_keys / 8];  // CHAR_BIT
  };
  KeyswitchScan curr_scan_;
  KeyswitchScan prev_scan_;

  // I'm not sure we need this conversion function. On the other hand, maybe it should be
  // public...
  LedAddr getLedAddr(KeyAddr key_addr) const;

  // special functions for Model01; make private if possible
  void enableHighPowerLeds();
  void enableScannerPower();
  boolean ledPowerFault();

  // This doesn't seem to be called anywhere
  void rebootBootloader();


  // --------------------------------------------------------------------------------
  // Iterator for range-based for loops
 public:
  class Iterator;
  friend class Keyboard::Iterator;

  Iterator begin() {
    return Iterator{*this, cKeyAddr::start};
  }
  Iterator end() {
    return Iterator{*this, cKeyAddr::end};
  }

  class Iterator {
   public:
    Iterator(Keyboard& keyboard, KeyAddr k) : keyboard_(keyboard), addr_(k.addr) {}

    bool operator!=(const Iterator& other);

    KeyswitchEvent& operator*();

    void operator++();

   private:
    Keyboard& keyboard_;
    byte addr_;
    KeyswitchEvent event_;

  }; // class Iterator {

}; // class Keyboard {


inline bool Keyboard::Iterator::operator!=(const Iterator& other) {
  byte r = addr_ / 8;

  while (addr_ < other.addr_) {
    if (keyboard_.curr_scan_.banks[r] != keyboard_.prev_scan_.banks[r]) {
      for (byte c = addr_ % 8; c < 8; ++c) {
        byte curr_state = bitRead(keyboard_.curr_scan_.banks[r], c);
        byte prev_state = bitRead(keyboard_.prev_scan_.banks[r], c);
        if (curr_state != prev_state) {
          addr_ = (r * 8) + c;
          event_.state = KeyswitchState(curr_state, prev_state);
          event_.addr = KeyAddr(addr_);
          event_.key = Key_NoKey;
          return true;
        }
      }
    }
    ++r;
    addr_ = r * 8;
  }
  return false;
}

inline KeyswitchEvent& Keyboard::Iterator::operator*() {
  return event_;
}

inline void Keyboard::Iterator::operator++() {
  // Copy current scan bit to previous scan bit
  // bitWrite(keyboard_.prev_scan_.banks[row_], col_,
  //          bitRead(keyboard_.curr_scan_.banks[row_], col_));
  ++addr_;
}

} // namespace model01 {
} // namespace kaleidoscope {
