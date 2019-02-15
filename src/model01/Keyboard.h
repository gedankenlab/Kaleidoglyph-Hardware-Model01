// -*- c++ -*-

#pragma once

#include <Arduino.h>

#include "model01/KeyswitchData.h"
#include "model01/Color.h"
#include "model01/LedAddr.h"
#include "model01/KeyAddr.h"
#include "model01/Scanner.h"

#include <kaleidoglyph/KeyState.h>
#include <kaleidoglyph/KeyEvent.h>
#include <kaleidoglyph/cKeyAddr.h>
#include <kaleidoglyph/cKey.h>


namespace kaleidoglyph {
namespace hardware {


class Keyboard {

 public:
  // This class should really be a singleton, but it probably costs a few bytes for the
  // extra getInstance() method that would be required to do that.
  Keyboard() : scanners_{Scanner(0), Scanner(3)} {}

  // New API
  void scanMatrix();

  // I really don't think we need this function, but maybe it will be useful
  KeyState keyswitchState(KeyAddr k) const;

  // Update all LEDs to values set by set*Color() functions below
  void updateLeds();

  void setAllLeds(Color color);
  void testLeds();

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
    Iterator(Keyboard& keyboard, KeyAddr k) : keyboard_(keyboard), addr_(byte(k)) {}

    bool operator!=(const Iterator& other);

    KeyEvent& operator*();

    void operator++();

   private:
    Keyboard& keyboard_;
    byte addr_;
    KeyEvent event_;

  }; // class Iterator {

}; // class Keyboard {


inline bool Keyboard::Iterator::operator!=(const Iterator& other) {
  // `r` stands for "row": really it's which set of eight keys we're testing, because the
  // key scan data is stored as a bitfield.
  byte r = addr_ / 8;  // 8 bits/byte

  // First, the test for the end condition (return false when all the keyswitches have
  // been tested)
  while (addr_ < other.addr_) {

    byte bank_prev = keyboard_.prev_scan_.banks[r];
    byte bank_curr = keyboard_.curr_scan_.banks[r];

    // Compare key scan data one byte at a time. The vast majority of the time, at most
    // one keyswitch will have changed states, so it's much more efficient to compare them
    // eight at a time, rather than one by one.
    if (bank_curr != bank_prev) {

      // When we find a row (byte) that has changed, we compare each bit until we find
      // each one that has changed state:
      for (byte c = addr_ % 8; c < 8; ++c) {  // 8 bits/byte

        bool curr_state = bitRead(bank_curr, c);
        bool prev_state = bitRead(bank_prev, c);

        if (curr_state != prev_state) {
          // We found a keyswitch that changed state, so we update the iterator's index
          // (`addr_`) and set the `event_` values accordingly before returning:
          addr_ = (r * 8) + c;

          event_.addr  = KeyAddr(addr_);
          event_.key   = cKey::blank;
          event_.state = KeyState(curr_state, prev_state);
          event_.caller = 0;

          // The `event_` will be returned by the dereference operator below, to be used
          // in the body of the loop:
          return true;
        } // if (curr_state != prev_state) {

      } // for (byte c = addr_ % 8; c < 8; ++c) { 

    }

    // We're done checking one "row" (byte); move on to the next one:
    ++r;
    addr_ = r * 8;

  } // while (addr_ < other.addr_) {
  return false;
}

inline KeyEvent& Keyboard::Iterator::operator*() {
  return event_;
}

inline void Keyboard::Iterator::operator++() {
  ++addr_;
}


#if 0
// To use the Keyboard::Iterator, write a loop like the following:
Keyboard keyboard;
for (KeyEvent event : keyboard) {
  // Here you'll get an `event` for each keyswitch that changed state in the current scan
  // cycle, and only those keyswitches, so most of the time the code in this block won't
  // execute at all.
}
#endif

} // namespace model01 {
} // namespace kaleidoglyph {
