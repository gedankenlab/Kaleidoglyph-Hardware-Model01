// -*- c++ -*-

#pragma once

#include <Arduino.h>

// Backward compatibility stuff here

#define HARDWARE_IMPLEMENTATION kaleidoscope::model01::Keyboard

#define COLS 16
#define ROWS 4

// End backcompat
#include "KeyswitchData.h"

// I think all we need is a forward declaration here
#include "Color.h"
#include "LedAddr.h"
#include "KeyAddr.h"
#include "Scanner.h"

namespace kaleidoscope {
namespace model01 {



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
  void setCrgbAt(byte row, byte col, cRGB color) {
    return setKeyColor(KeyAddr(row * 8 + col), color);
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

  void actOnMatrixScan(void); // private!

  // These are only used by TestMode & MagicCombo
  // keydata_t leftHandState;
  // keydata_t rightHandState;
  // keydata_t previousLeftHandState;
  // keydata_t previousRightHandState;

  // End backcompat


  // New API
  void scanMatrix();

#if 0
  // should probably return KeyswitchEvent instead
  byte nextKeyswitchEvent(KeyAddr key_addr);
  // Return the physical keyswitch state
  byte keyswitchState(KeyAddr key_addr) const;
  // Return the keyswitch state as a boolean (true == pressed)
  bool isKeyswitchPressed(KeyAddr key_addr) const;
#endif
  // Update all LEDs to values set by set*Color() functions below
  void updateLeds();

  // These functions operate on LedAddr values, which are different from corresponding KeyAddr values
  Color const & getLedColor(LedAddr led_addr) const;
  void setLedColor(LedAddr led_addr, Color color);

  // These are the KeyAddr versions, which call the LedAddr functions
  Color const & getKeyColor(KeyAddr key_addr) const;
  void setKeyColor(KeyAddr key_addr, Color color);

  // I'm leaving these functions alone for now; they shall remain mysterious
  void setup();

  // This function is used by TestMode
  void setKeyscanInterval(uint8_t interval);

 private:
  static constexpr byte HAND_BIT = B00100000;

  Scanner scanners_[2];

  union KeyboardState {
    KeyswitchData hands[2];
    byte banks[total_keys / 8];  // CHAR_BIT
  };
  KeyboardState keyboard_state_;
  KeyboardState prev_keyboard_state_;

  union KeyswitchStates {
    KeyswitchData hands[2];
    byte banks[total_keys / 8];  // CHAR_BIT
  };
  KeyboardState curr_scan_;
  KeyboardState prev_scan_;

  // I'm not sure we need this conversion function. On the other hand, maybe it should be
  // public...
  LedAddr getLedAddr(KeyAddr key_addr) const;

  // special functions for Model01; make private if possible
  void enableHighPowerLeds();
  void enableScannerPower();
  boolean ledPowerFault();

  // This doesn't seem to be called anywhere
  void rebootBootloader();

 private:
  class Iterator;
  friend class Keyboard::Iterator;

 public:
  Iterator begin() const {
    return Iterator{*this, cKeyAddr::start};
  }
  Iterator end() const {
    return Iterator{*this, cKeyAddr::end}
  }

  // Very strange iterator where almost everything is done in the comparison. We iterate
  // through the key addresses, but 8 keys (a "bank") at a time, until we find a bank
  // that's changed since the previous scan. Then, we check the bits one at a time, until
  // we find the first one that changed, and we construct a partial KeyswitchEvent object
  // (for the Controller to complete), which will be returned by the iterator in the
  // dereference called when this returns true. On the next pass addr will be incremented,
  // which has the correct affect on both the other members of the union, so we start
  // checking with the next keyswitch.
  class Iterator {
   public:
    Iterator(Keyboard& keyboard, KeyAddr addr)
        : keyboard_(keyboard), addr_(addr.addr()), event{} {}

    bool operator!=(Iterator& other) const {
      // First, the actual end condition test:
      while (addr != other.addr) {
        // Compare the next bank (r). If it hasn't changed, skip ahead to the next one:
        if (keyboard_.curr_scan_.banks[r] == keyboard_.prev_scan_.banks[r]) {
          ++r;
          addr = r << 3;
          continue;
        }
        // We found a bank that changed. Test the current bit:
        do {
          byte curr_state = bitRead(keyboard_.curr_scan_.banks[r], c);
          byte prev_state = bitRead(keyboard_.prev_scan_.banks[r], c);
          if (curr_state == prev_state) { // swap this condition, and the two blocks following
            // If this bit hasn't changed, check the next one:
            ++addr; // Incrementing addr also increments c (which overflows to r)
          } else {
            // We found a bit that changed; get the event ready and return true:
            event.state = KeyswitchState(curr_state, prev_state);
            event.addr = addr;
            bitWrite(keyboard_.prev_scan_.banks[r], c, curr_state);
            return true;
          }
          // If c == 0, that means we've reached the end of the current bank, and can go
          // on to the next one (starting back at the beginning of the enclosing loop)
        } while (c != 0);
      } // while (addr != other.addr) {
      return false;
    }

    KeyswitchEvent& operator*() {
      return event; // reference, probably
    }

    void operator++() {
      ++addr; // also affects r & c
    }

   private:
    Keyboard& keyboard_;
    union {
      byte addr;
      byte c : 3, r : 3;
    };
    KeyswitchEvent event;
  };
};

} // namespace model01 {
} // namespace kaleidoscope {
