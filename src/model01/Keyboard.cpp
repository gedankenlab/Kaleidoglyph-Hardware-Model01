#include "Keyboard.h"

#include <Arduino.h>
#include <avr/wdt.h>

#include "model01/Color.h"
#include "model01/KeyAddr.h"
#include "model01/LedAddr.h"
#include "model01/Scanner.h"

#include <kaleidoscope/KeyswitchState.h>
#include <kaleidoscope/KeyswitchEvent.h>


namespace kaleidoscope {
namespace hardware {


void Keyboard::scanMatrix() {

  // I'm tempted to abuse this function and include the LED updating here, but it should
  // probably be done the hard way with a plugin. I'm going to do it anyway to simplify
  // things for now, but I'll leave this comment here as a reminder to do it right
  // eventually. The simplest thing to do is just call updateLeds(), but a timer should
  // really be used to keep them from updating too often.
  updateLeds();

  // copy current keyswitch state array to previous
  //memcpy(&prev_scan_, &curr_scan_, sizeof(prev_scan_));
  prev_scan_ = curr_scan_;

  // scan left hand
  scanners_[0].readKeys(curr_scan_.hands[0]);

  // scan right hand
  scanners_[1].readKeys(curr_scan_.hands[1]);
}

// return the state of the keyswitch as a bitfield
KeyswitchState Keyboard::keyswitchState(KeyAddr k) const {
  byte state = 0;
  byte r = byte(k) / 8;
  byte c = byte(k) % 8;
  return KeyswitchState(bitRead(curr_scan_.banks[r], c),
                        bitRead(prev_scan_.banks[r], c));
}


static constexpr byte HAND_BIT = B00100000;
static constexpr byte LED_BITS = B00011111;

Color Keyboard::getLedColor(LedAddr led) const {
  bool hand = byte(led) & HAND_BIT; // B00100000
  return scanners_[hand].getLedColor(byte(led) & LED_BITS);
}

void Keyboard::setLedColor(LedAddr led, Color color) {
  bool hand = byte(led) & HAND_BIT; // B00100000
  scanners_[hand].setLedColor(byte(led) & LED_BITS, color);
  scanners_[hand].updateLed(byte(led) & LED_BITS, color);
}

Color Keyboard::getKeyColor(KeyAddr k) const {
  return getLedColor(LedAddr{k});
}

void Keyboard::setKeyColor(KeyAddr k, Color color) {
  setLedColor(LedAddr{k}, color);
}


// Update one bank of LEDs at a time, until they've all been updated, then reset. Updates
// will begin happening after a fixed number of milliseconds (`led_update_interval`) have
// elapsed since the previous update began.
void Keyboard::updateLeds() {
  static byte t0{0};
  static byte next_led_bank{0};
  static constexpr byte total_led_banks{4};
  static constexpr byte led_update_interval{32}; // milliseconds between frames (~30 fps)
  if (next_led_bank < total_led_banks) {
    scanners_[0].updateLedBank(next_led_bank);
    scanners_[1].updateLedBank(next_led_bank);
    ++next_led_bank;
  } else if (millis() - t0 > led_update_interval) {
    t0 += led_update_interval;
    next_led_bank = 0;
    // This is where led-mode plugins should get called to update the led values
  }
}

void Keyboard::setAllLeds(Color color) {
  scanners_[0].updateAllLeds(color);
  scanners_[1].updateAllLeds(color);
}

// My question here is why this is done in a separate setup() function; I suppose it's
// because we need other objects to start up before calling functions that affect the
// scanners
void Keyboard::setup() {
  wdt_disable();
  delay(100);
  enableScannerPower();

  // Consider not doing this until 30s after keyboard
  // boot up, to make it easier to rescue things
  // in case of power draw issues.
  enableHighPowerLeds();
  memset(&curr_scan_, 0, sizeof(curr_scan_));
  memset(&prev_scan_, 0, sizeof(prev_scan_));

  TWBR = 12; // This is 400mhz, which is the fastest we can drive the ATTiny

  // Turn off all LEDs at startup. TODO: move this elsewhere?
  scanners_[0].updateAllLeds(Color{0,0,0});
  scanners_[1].updateAllLeds(Color{0,0,0});

  scanners_[0].testLeds();
  scanners_[1].testLeds();
}

// why extern "C"? Because twi.c is not C++!
extern "C" {
#include "twi/twi.h"
}
#include "twi/wire-protocol-constants.h"
void Keyboard::testLeds() {
  // for (int i{0}; i < 32; ++i) {
  //   scanners_[0].updateLed(i, {180, 50, 250});
  //   delay(3);
  //   scanners_[1].updateLed(i, {250, 180, 50});
  //   delay(200);
  //   scanners_[0].updateLed(i, {0, 0, 0});
  //   delay(3);
  //   scanners_[1].updateLed(i, {0, 0, 0});
  //   delay(200);
  // }

  byte results[8];
  byte r{0};
  // TODO: send hard-coded led bank updates
  byte data[24 + 1];
  byte i{0};
  data[i] = TWI_CMD_LED_BASE;
  while (i < (sizeof(data) - 1)) {
    data[++i] = 0;
    data[++i] = 250;
    data[++i] = 100;
  }
  byte data2[] = { TWI_CMD_LED_BASE,
                   250, 0, 100,
                   250, 0, 100,
                   250, 0, 100,
                   250, 0, 100,
                   250, 0, 100,
                   250, 0, 100,
                   250, 0, 100,
                   250, 0, 100,
  };

  uint32_t t0 = micros();

  results[r++] = twi_writeTo(0x58, data, sizeof(data), 1, 0);
  results[r++] = twi_writeTo(0x58 + 3, data2, sizeof(data2), 1, 0);

  ++data[0];
  results[r++] = twi_writeTo(0x58, data, sizeof(data), 1, 0);
  ++data2[0];
  results[r++] = twi_writeTo(0x58 + 3, data2, sizeof(data), 1, 0);

  ++data[0];
  results[r++] = twi_writeTo(0x58, data, sizeof(data), 1, 0);
  ++data2[0];
  results[r++] = twi_writeTo(0x58 + 3, data2, sizeof(data), 1, 0);

  ++data[0];
  results[r++] = twi_writeTo(0x58, data, sizeof(data), 1, 0);
  ++data2[0];
  results[r++] = twi_writeTo(0x58 + 3, data2, sizeof(data), 1, 0);

  uint32_t t1 = micros();

  // delay(1000);
  for (byte i{0}; i < r; ++i) {
    Serial.print(int(results[i])), Serial.print(F(", "));
  }
  Serial.println();
  delay(10);
  Serial.println(t1 - t0);

  // for (int bank{0}; bank < 4; ++bank) {
  //   for (int i{0}; i < 8; ++i) {
  //     byte led = bank * 8 + i;
  //     scanners_[0].setLedColor(led, {100, 100, 240});
  //   }
  //   scanners_[0].updateLedBank(bank);
  //   Serial.println(int(bank));
  //   delay(500);
  // }

  // scanners_[0].updateAllLeds({0,0,0});
  // scanners_[1].updateAllLeds({0,0,0});
}


void Keyboard::enableScannerPower() {
  // PC7
  //pinMode(13, OUTPUT);
  //digitalWrite(13, HIGH);
  // Turn on power to the LED net
  DDRC |= _BV(7);
  PORTC |= _BV(7);
}


// This lets the keyboard pull up to 1.6 amps from
// the host. That violates the USB spec. But it sure
// is pretty looking
void Keyboard::enableHighPowerLeds() {
  // PE6
  //    pinMode(7, OUTPUT);
  //    digitalWrite(7, LOW);
  DDRE |= _BV(6);
  PORTE &= ~_BV(6);

  // Set B4, the overcurrent check to an input with an internal pull-up
  DDRB &= ~_BV(4);	// set bit, input
  PORTB &= ~_BV(4);	// set bit, enable pull-up resistor
}


boolean Keyboard::ledPowerFault() {
  if (PINB & _BV(4)) {
    return true;
  } else {
    return false;
  }
}


void Keyboard::rebootBootloader() {
  // Set the magic bits to get a Caterina-based device
  // to reboot into the bootloader and stay there, rather
  // than run move onward
  //
  // These values are the same as those defined in
  // Caterina.c

  uint16_t boot_key = 0x7777;
  uint16_t* const boot_key_ptr = reinterpret_cast<uint16_t *>(0x0800);

  // Stash the magic key
  *boot_key_ptr = boot_key;

  // Set a watchdog timer
  wdt_enable(WDTO_120MS);

  while (1) {} // This infinite loop ensures nothing else
  // happens before the watchdog reboots us
}


void Keyboard::setKeyscanInterval(uint8_t interval) {
  scanners_[0].setKeyscanInterval(interval);
  scanners_[1].setKeyscanInterval(interval);
}

} // namespace hardware {
} // namespace kaleidoscope {
