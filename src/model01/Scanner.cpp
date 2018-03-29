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

#include "model01/Scanner.h"

#include <Arduino.h>

#include "model01/Color.h"
#include "model01/KeyswitchData.h"


// why extern "C"? Because twi.c is not C++!
extern "C" {
#include "twi/twi.h"
}
#include "twi/wire-protocol-constants.h"


namespace kaleidoscope {
namespace hardware {

// Magic constant with no documentation...
constexpr byte SCANNER_I2C_ADDR_BASE = 0x58;
// Can't figure out a simple way of making this type-safe & constexpr. Since it's confined
// to this file, it's not that important.
#define ELEMENTS(array)  (sizeof(array) / sizeof((array)[0]))

// This array translates LED values to corrected values. It can be cut in half, or even to
// 1/4th the size, if we take a performance hit by bit-shifting the values before
// translation. 64 different brightness levels should be plenty.
const PROGMEM byte gamma8[] = {
  0,   0,
  0,   1,
  1,   2,
  3,   5,
  7,   10,
  13,  16,
  20,  25,
  30,  36,
  43,  50,
  59,  68,
  78,  89,
  101, 114,
  127, 142,
  158, 175,
  193, 213,
  233, 255,
};


// Constructor
Scanner::Scanner(byte ad01) {
  ad01_ = ad01;
  addr_ = SCANNER_I2C_ADDR_BASE | ad01_;
  // I think twi_init() just sets things up on the controller, so it only gets called
  // once. Maybe this shouldn't be in the constructor, but in an init() method instead.
  static bool twi_uninitialized = true;
  if (twi_uninitialized) {
    twi_init();
    twi_uninitialized = false;
  }
}

// This function should just return a KeyswitchData object, and not bother storing it as a
// member of the Scanner object. This reference parameter needs testing to see if it works
// as I expect.
bool Scanner::readKeys(KeyswitchData &key_data) {
  byte rx_buffer[sizeof(key_data) + 1];

  // perform blocking read into buffer
  byte read = twi_readFrom(addr_, rx_buffer, ELEMENTS(rx_buffer), true);
  if (rx_buffer[0] == TWI_REPLY_KEYDATA) {
    // memcpy(&key_data, &rx_buffer[1], sizeof(key_data));
    for (byte i = 0; i < sizeof(key_data); ++i) {
      key_data.banks[i] = rx_buffer[i + 1];
    }
    return true;
  } else {
    return false;
  }
}


const Color& Scanner::getLedColor(byte led) const {
  //assert(led < LEDS_PER_HAND);
  return led_states_.leds[led];
}

void Scanner::setLedColor(byte led, Color color) {
  //assert(led < LEDS_PER_HAND);
  if (led_states_.leds[led] != color) {
    led_states_.leds[led] = color;
    byte bank = led / LEDS_PER_BANK;
    bitSet(led_banks_changed_, bank);
  }
}


// This function gets called to set led status on one bank (eight LEDs) at a time. Each
// time it's called, it updates the next bank. I'm renaming it to be more clear.
void Scanner::updateNextLedBank() {
  updateLedBank(next_led_bank_++);
  if (next_led_bank_ == total_led_banks_) {
    next_led_bank_ = 0;
  }
}


// This function is private, and only gets called by updateNextLedBank() (see above)
void Scanner::updateLedBank(byte bank) {
  if (! bitRead(led_banks_changed_, bank))
    return;
  byte data[led_bytes_per_bank_ + 1];
  byte i{0};
  data[i] = TWI_CMD_LED_BASE + bank;
  for (Color color : led_states_.led_banks[bank]) {
    data[++i] = pgm_read_byte(&gamma8[color.b()]);
    data[++i] = pgm_read_byte(&gamma8[color.g()]);
    data[++i] = pgm_read_byte(&gamma8[color.r()]);
  }
  byte result = twi_writeTo(addr_, data, ELEMENTS(data), 1, 0);
  bitClear(led_banks_changed_, bank);
}


// An efficient way to set the value of just one LED, without having to update everything
void Scanner::updateLed(byte led, Color color) {
  byte data[] = {TWI_CMD_LED_SET_ONE_TO,
                 led,
                 pgm_read_byte(&gamma8[color.b()]),
                 pgm_read_byte(&gamma8[color.g()]),
                 pgm_read_byte(&gamma8[color.r()])
                };
  byte result = twi_writeTo(addr_, data, ELEMENTS(data), 1, 0);
  led_states_.leds[led] = color;
}


// An efficient way to set all LEDs to the same color at once
void Scanner::updateAllLeds(Color color) {
  byte data[] = {TWI_CMD_LED_SET_ALL_TO,
                 pgm_read_byte(&gamma8[color.b()]),
                 pgm_read_byte(&gamma8[color.g()]),
                 pgm_read_byte(&gamma8[color.r()])
                };
  byte result = twi_writeTo(addr_, data, ELEMENTS(data), 1, 0);
  // we should set all the values of led_states_ here
  for (byte led = 0; led < leds_per_hand_; ++led) {
    led_states_.leds[led] = color;
  }
  led_banks_changed_ = 0;
}


// Sets the keyscan interval. We currently do three reads.
// before declaring a key event debounced.
//
// Takes an integer value representing a counter.
//
// 0 - 0.1-0.25ms
// 1 - 0.125ms
// 10 - 0.35ms
// 25 - 0.8ms
// 50 - 1.6ms
// 100 - 3.15ms
//
// You should think of this as the _minimum_ keyscan interval.
// LED updates can cause a bit of jitter.
//
// returns the Wire.endTransmission code (0 = success)
// https://www.arduino.cc/en/Reference/WireEndTransmission
byte Scanner::setKeyscanInterval(byte delay) {
  byte data[] = {TWI_CMD_KEYSCAN_INTERVAL, delay};
  byte result = twi_writeTo(addr_, data, ELEMENTS(data), 1, 0);

  return result;
}


// These functions seem to be here only for debugging purposes, and can probably be removed

// This is called from other debugging functions
byte Scanner::readRegister(byte cmd) {
  byte return_value = 0;

  byte data[] = {cmd};
  byte result = twi_writeTo(addr_, data, ELEMENTS(data), 1, 0);

  delayMicroseconds(15); // We may be able to drop this in the future
  // but will need to verify with correctly
  // sized pull-ups on both the left and right
  // hands' i2c SDA and SCL lines

  byte rx_buffer[1];

  // perform blocking read into buffer
  byte read = twi_readFrom(addr_, rx_buffer, ELEMENTS(rx_buffer), true);
  if (read > 0) {
    return rx_buffer[0];
  } else {
    return -1;
  }
}

// returns -1 on error, otherwise returns the scanner version integer
byte Scanner::readVersion() {
  return readRegister(TWI_CMD_VERSION);
}

// returns -1 on error, otherwise returns the scanner keyscan interval
byte Scanner::readKeyscanInterval() {
  return readRegister(TWI_CMD_KEYSCAN_INTERVAL);
}

// returns -1 on error, otherwise returns the LED SPI Frequncy
byte Scanner::readLedSpiFrequency() {
  return readRegister(TWI_CMD_LED_SPI_FREQUENCY);
}

// The only thing that uses this right now is the RainbowWave plugin

// Set the LED SPI Frequency. See wire-protocol-constants.h for
// values.
//
// returns the Wire.endTransmission code (0 = success)
// https://www.arduino.cc/en/Reference/WireEndTransmission
byte Scanner::setLedSpiFrequency(byte frequency) {
  byte data[] = {TWI_CMD_LED_SPI_FREQUENCY, frequency};
  byte result = twi_writeTo(addr_, data, ELEMENTS(data), 1, 0);

  return result;
}

} // namespace hardware {
} // namespace kaleidoscope {
