// -*- c++ -*-

/*
The MIT License (MIT)

Copyright © 2015 Jesse Vincent <jesse@fsck.com>
Copyright © 2018 Michael Richters <gedankenexperimenter@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <Arduino.h>

#include "model01/KeyswitchData.h"
#include "model01/Color.h"

// See .cpp file for comments regarding appropriate namespaces
namespace kaleidoscope {
namespace hardware {

// I'm uncertain of how to manage these constants
constexpr byte LEDS_PER_BANK = 8;
constexpr byte TOTAL_LEDS    = 32;

// used to configure interrupts, configuration for a particular controller
class Scanner {
 public:
  Scanner(byte ad01);

  bool readKeys(KeyswitchData &key_data);

  // only here for debugging, I think; it's unused in firmware
  byte readVersion();

  byte setKeyscanInterval(byte delay);
  byte readKeyscanInterval();

  byte setLedSpiFrequency(byte frequency);
  byte readLedSpiFrequency();

  // interface to LED color array
  const Color& getLedColor(byte i) const;
  void setLedColor(byte i, Color color);

  // send message to controller to change physical LEDs
  void updateNextLedBank();

  void updateLed(byte led, Color color);
  void updateAllLeds(Color color);

 private:
  byte addr_;
  byte ad01_;

  byte readRegister(byte cmd);

  // These constants might be wasting some space vs #define
  static constexpr byte total_leds_         = TOTAL_LEDS;  // per controller
  static constexpr byte leds_per_hand_      = TOTAL_LEDS / 2;
  static constexpr byte leds_per_bank_      = LEDS_PER_BANK;   // CHAR_BIT
  static constexpr byte total_led_banks_    = TOTAL_LEDS / LEDS_PER_BANK;
  static constexpr byte led_bytes_per_bank_ = LEDS_PER_BANK * sizeof(Color);

  // This union stores the (pending) color data for all the LEDs controlled by this
  // scanner/controller
  union {
    Color leds[leds_per_hand_];
    Color led_banks[total_led_banks_][leds_per_bank_];
    byte banks[total_led_banks_][led_bytes_per_bank_];
  } led_states_;

  // the next LED bank that will be updated by updateLedBank()
  byte next_led_bank_ = 0;

  // bitfield storing which LED banks need an update
  byte led_banks_changed_ = 0;

  void updateLedBank(byte bank);

}; // class Scanner {

} // namespace hardware {
} // namespace kaleidoscope {
