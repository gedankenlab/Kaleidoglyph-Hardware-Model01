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
#include <assert.h>

#include "model01/Color.h"
#include "model01/KeyswitchData.h"
#include <kaleidoglyph/utils.h>

// why extern "C"? Because twi.c is not C++!
extern "C" {
#include "twi/twi.h"
}
#include "twi/wire-protocol-constants.h"


namespace kaleidoglyph {
namespace hardware {

// Magic constant with no documentation...
constexpr byte SCANNER_I2C_ADDR_BASE = 0x58;

// This array translates LED values to corrected values. It can be cut in half, or even to
// 1/4th the size, if we take a performance hit by bit-shifting the values before
// translation. 64 different brightness levels should be plenty.
const PROGMEM byte gamma8[] = {
  0,   1,
  2,   3,
  4,   5,
  7,   8,
  10,  11,
  13,  15,
  18,  22,
  25,  32,
  40,  49,
  59,  68,
  78,  89,
  101, 114,
  127, 142,
  158, 175,
  193, 213,
  233, 255,
};

static bool twi_uninitialized = true;

// Constructor
Scanner::Scanner(byte ad01) {
  ad01_ = ad01;
  addr_ = SCANNER_I2C_ADDR_BASE | ad01_;
  // I think twi_init() just sets things up on the controller, so it only gets called
  // once. Maybe this shouldn't be in the constructor, but in an init() method instead.
  if (twi_uninitialized) {
    twi_init();
    twi_uninitialized = false;
  }
}


void Scanner::testLeds() {
  Color bright{150, 200, 250};
  Color off{0, 0, 0};
  KeyswitchData kd;
  for (byte i{0}; i < 32; ++i) {
    uint16_t t0 = micros();
    updateLed(i, bright);
    uint16_t t1 = micros();
    // readKeys here avoids the bug where sending two consecutive writes to the same
    // scanner returns an i2c error (address NACK)
    readKeys(kd);
    Serial.print(F("LED update time: "));
    Serial.print(t1 - t0);
    Serial.println(F(" µs"));
    delay(100);
    updateLed(i, off);
    readKeys(kd);
    delay(50);
  }
  for (byte i{0}; i < leds_per_hand_; ++i) {
    byte r = 2 * i;
    byte g = 6 * i;
    byte b = i + 70;
    setLedColor(i, Color{r, g, b});
  }
  for (byte i{0}; i < 4; ++i) {
    uint16_t t0 = micros();
    updateNextLedBank();
    uint16_t t1 = micros();
    // uint16_t t2;
    // for (byte j{0}; j < 10; ++j) {
    //   t2 = micros();
    // }
    Serial.print(F("LED bank update time: "));
    Serial.print(t1 - t0);
    Serial.println(F(" µs"));
    // Serial.print(F("micros() time: "));
    // Serial.print(t2 - t1);
    // Serial.println(F(" µs"));
    delay(1000);
    Serial.print(F("Reading keyswitch data: "));
    t0 = micros();
    readKeys(kd);
    t1 = micros();
    Serial.print(t1 - t0);
    Serial.println(F(" µs"));
    delay(1000);
  }
}

// This function should just return a KeyswitchData object, and not bother storing it as a
// member of the Scanner object. This reference parameter needs testing to see if it works
// as I expect.
bool Scanner::readKeys(KeyswitchData& key_data) {
  byte rx_buffer[sizeof(key_data) + 1];

  // perform blocking read into buffer
  /*byte read =*/ twi_readFrom(addr_, rx_buffer, arraySize(rx_buffer), true);
  if (rx_buffer[0] == TWI_REPLY_KEYDATA) {
    // memcpy(&key_data, &rx_buffer[1], sizeof(key_data));
    for (byte i{0}; i < sizeof(key_data); ++i) {
      key_data.banks[i] = rx_buffer[i + 1];
    }
    return true;
  } else {
    return false;
  }
}


Color Scanner::getLedColor(byte led) const {
  //assert(led < LEDS_PER_HAND);
  // byte bank = led / LEDS_PER_BANK;
  // led %= LEDS_PER_BANK;
  // return led_states_[bank][led];
  return led_colors_[led];
}

void Scanner::setLedColor(byte led, Color color) {
  //assert(led < LEDS_PER_HAND);
  // byte bank = led / LEDS_PER_BANK;
  // led %= LEDS_PER_BANK;
  // if (led_states_[bank][led] != color) {
  //   led_states_[bank][led] = color;
  //   bitSet(led_banks_changed_, bank);
  // }
  if (led_colors_[led] != color) {
    led_colors_[led] = color;
    byte bank = led / leds_per_bank_;
    bitSet(led_banks_changed_, bank);
  }
}


// This function gets called to set led status on one bank (eight LEDs) at a time. Each
// time it's called, it updates the next bank. I'm renaming it to be more clear.
void Scanner::updateNextLedBank() {
  if (next_led_bank_ >= total_led_banks_) {
    next_led_bank_ = 0;
  }
  updateLedBank(next_led_bank_++);
}


// This function is private, and only gets called by updateNextLedBank() (see above)
void Scanner::updateLedBank(byte bank) {
  // TODO: make this assert do something useful
  assert(bank < total_led_banks_);
  if (! bitRead(led_banks_changed_, bank))
    return;
  byte data[led_bytes_per_bank_ + 1];
  data[0] = TWI_CMD_LED_BASE + bank;
  byte led = bank * leds_per_bank_;
  // I had a bug where we were running off the end of this array. It might still be
  // there. It might not actually be here, but in the caller, because `bank` might be out
  // of bounds.
  for (byte i{0}; i <= led_bytes_per_bank_ - 3;) {
    Color color = led_colors_[led++];
    data[++i] = pgm_read_byte(&gamma8[color.b()]);
    data[++i] = pgm_read_byte(&gamma8[color.g()]);
    data[++i] = pgm_read_byte(&gamma8[color.r()]);
  }
  // for (Color color : led_states_[bank]) {
  //   data[++i] = pgm_read_byte(&gamma8[color.b()]);
  //   data[++i] = pgm_read_byte(&gamma8[color.g()]);
  //   data[++i] = pgm_read_byte(&gamma8[color.r()]);
  // }
  // TODO: get rid of this delay
  //delay(5);
  //byte result =
  twi_writeTo(addr_, data, sizeof(data), 1, 0);
  // while (byte result = twi_writeTo(addr_, data, sizeof(data), 1, 0)) {
  //   Serial.print(int(bank)), Serial.print(F(","));
  //   Serial.print(int(led)), Serial.print(F(": "));
  //   Serial.println(int(result));
  //   // delay(5);
  // }
  // Serial.print(F("Updated LED bank "));
  // Serial.print(int(bank));
  // Serial.print(F(": return code = "));
  // Serial.println(int(result));
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
  while (byte result = twi_writeTo(addr_, data, arraySize(data), 1, 0)) {
    Serial.print(int(led)); Serial.print(F(": ")); Serial.println(int(result));
    // TODO: fix this so we don't need the delay at all. It may be that the best way is to
    // guarantee that it only gets called once per scan cycle, or that updates are only
    // allowed by whole banks or all LEDs at once.
    //delay(2);
  }
  led_colors_[led] = color;
  // byte bank = led / LEDS_PER_BANK;
  // led %= LEDS_PER_BANK;
  // led_states_[bank][led] = color;
}


// An efficient way to set all LEDs to the same color at once
void Scanner::updateAllLeds(Color color) {
  byte data[] = {TWI_CMD_LED_SET_ALL_TO,
                 pgm_read_byte(&gamma8[color.b()]),
                 pgm_read_byte(&gamma8[color.g()]),
                 pgm_read_byte(&gamma8[color.r()])
                };
  byte result = twi_writeTo(addr_, data, arraySize(data), 1, 0);
  if (result != 0) return;

  // for (byte led{0}; led < leds_per_hand_; ++led) {
  //   led_colors_[led] = color;
  // }

  for (Color& c : led_colors_) {
    c = color;
  }

  // we should set all the values of led_states_ here
  // for (byte bank{0}; bank < total_led_banks_; ++bank) {
  //   for (byte led{0}; led < leds_per_bank_; ++led) {
  //     led_states_[bank][led] = color;
  //   }
  // }

  // This one is the most efficient non-union version
  // for (byte bank{0}; bank < total_led_banks_; ++bank) {
  //   for (Color& c : led_states_[bank]) {
  //     c = color;
  //   }
  // }

  // for (auto& led_bank : led_states_) {
  //   for (Color& c : led_bank) {
  //     c = color;
  //   }
  // }

  // for (byte led = 0; led < leds_per_hand_; ++led) {
  //   led_states_.leds[led] = color;
  // }

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
  byte result = twi_writeTo(addr_, data, arraySize(data), 1, 0);

  return result;
}


// These functions seem to be here only for debugging purposes, and can probably be removed

// This is called from other debugging functions
byte Scanner::readRegister(byte cmd) {
  byte data[] = {cmd};
  /*byte result =*/ twi_writeTo(addr_, data, arraySize(data), 1, 0);

  delayMicroseconds(15); // We may be able to drop this in the future
  // but will need to verify with correctly
  // sized pull-ups on both the left and right
  // hands' i2c SDA and SCL lines

  byte rx_buffer[1];

  // perform blocking read into buffer
  byte read = twi_readFrom(addr_, rx_buffer, arraySize(rx_buffer), true);
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
  byte result = twi_writeTo(addr_, data, arraySize(data), 1, 0);

  return result;
}

} // namespace hardware {
} // namespace kaleidoglyph {
