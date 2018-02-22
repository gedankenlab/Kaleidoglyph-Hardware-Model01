#include "Keyboard.h"

#include <Arduino.h>
#include <avr/wdt.h>
#include <stdint.h>

// backcompat
#include <Kaleidoscope.h>

#include "Color.h"
#include "KeyAddr.h"
#include "LedAddr.h"
#include "Scanner.h"


namespace kaleidoscope {
namespace model01 {

// Why don't we do these things in the constructor? Why are they static? There's only one object...
// static Scanner scanners[] = {
//   Scanner(0), Scanner(1)
// };

//Keyboard::scanners_[0] = Scanner(0);
//Keyboard::scanners_[1] = Scanner(3);

// *INDENT-OFF*
static constexpr PROGMEM uint8_t key_led_map[total_keys] = {
  27, 26, 20, 19, 12, 11,  4,  3,
  28, 25, 21, 18, 13, 10,  5,  2,
  29, 24, 22, 17, 14,  9,  6,  1,
  30, 31, 23, 16, 15,  8,  7,  0,

  60, 59, 52, 51, 44, 43, 37, 36,
  61, 58, 53, 50, 45, 42, 38, 35,
  62, 57, 54, 49, 46, 41, 39, 34,
  63, 56, 55, 48, 47, 40, 32, 33,
};
// *INDENT-ON*

Keyboard::Keyboard() : scanners_{Scanner(0), Scanner(3)} {}


void Keyboard::scanMatrix() {
  // copy current keyswitch state array to previous
  memcpy(&prev_keyboard_state_, &keyboard_state_, sizeof(prev_keyboard_state_));

  // scan left hand
  scanners_[0].readKeys(keyboard_state_.hands[0]);

  // scan right hand
  scanners_[1].readKeys(keyboard_state_.hands[1]);

  // backcompat
  actOnMatrixScan();
}

// backcompat
void Keyboard::actOnMatrixScan() {
  for (byte hand = 0; hand < 2; hand++) {
    for (byte row = 0; row < 4; row++) {
      for (byte col = 0; col < 8; col++) {
        byte bank = (hand << 0) | (row << 1);
        byte key_state = ((bitRead(keyboard_state_.banks[bank], col) << 1) |
                          (bitRead(prev_keyboard_state_.banks[bank], col) << 0));
        handleKeyswitchEvent(Key_NoKey, row, (hand << 3) | col, key_state);
      }
    }
  }
}

#if 0
// get the address of the next key that changed state (if any), starting from the last one
// checked (or at least, that's the expected usage). Return true if we found an event;
// false if we didn't find a keyswitch in a different state. Maybe it should return the
// state value instead.
byte Keyboard::nextKeyswitchEvent(KeyAddr& key_addr) {
  // compare state bitfield one byte at a time until we find one that differs
  for (byte r = (key_addr / 8); r < 8; ++r) {
    if (keyboard_state_[r] == prev_keyboard_state_[r]) {
      continue;
    }
    // next compare the bits one at a time
    for (byte c = (key_addr % 8); c < 8; ++c) {
      byte prev_state = bitRead(prev_keyboard_state[r], c);
      byte curr_state = bitRead(keyboard_state_[r], c);
      if (prev_state != curr_state) {
        key_addr = (r << 3) | c;
        return (prev_state << 1) | curr_state;
      }
    }
  }
  // key_addr = Keyboard::total_keys;
  return 0;
}


// return the state of the keyswitch as a bitfield
byte Keyboard::keyswitchState(KeyAddr key_addr) const {
  byte state = 0;
  byte r = key_addr / 8;
  byte c = key_addr % 8;
  state |= bitRead(prev_keyboard_state_[r], c) << 1;
  state |= bitRead(keyboard_state_[r], c);
  return state;
}
#endif

constexpr byte HAND_BIT = B00100000;

LedAddr Keyboard::getLedAddr(KeyAddr key_addr) const {
  return LedAddr(pgm_read_byte(&(key_led_map[key_addr.addr()])));
}

const Color& Keyboard::getLedColor(LedAddr led_addr) const {
  bool hand = led_addr & HAND_BIT; // B00100000
  return scanners_[hand].getLedColor(led_addr & ~HAND_BIT);
}

void Keyboard::setLedColor(LedAddr led_addr, Color color) {
  bool hand = led_addr & HAND_BIT; // B00100000
  scanners_[hand].setLedColor(led_addr & ~HAND_BIT, color);
}

const Color& Keyboard::getKeyColor(KeyAddr key_addr) const {
  LedAddr led_addr = getLedAddr(key_addr);
  return getLedColor(led_addr);
}

void Keyboard::setKeyColor(KeyAddr key_addr, Color color) {
  LedAddr led_addr = getLedAddr(key_addr);
  setLedColor(led_addr, color);
}


// This function is a bit better now, but I still feel the desire to write this as an
// explicit loop
void Keyboard::updateLeds() {
  scanners_[0].updateNextLedBank();
  scanners_[1].updateNextLedBank();

  scanners_[0].updateNextLedBank();
  scanners_[1].updateNextLedBank();

  scanners_[0].updateNextLedBank();
  scanners_[1].updateNextLedBank();

  scanners_[0].updateNextLedBank();
  scanners_[1].updateNextLedBank();
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
  memset(&keyboard_state_, 0, sizeof(keyboard_state_));
  memset(&prev_keyboard_state_, 0, sizeof(prev_keyboard_state_));

  TWBR = 12; // This is 400mhz, which is the fastest we can drive the ATTiny
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
  uint16_t *const boot_key_ptr = reinterpret_cast<uint16_t *>(0x0800);

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

// backcompat
HARDWARE_IMPLEMENTATION KeyboardHardware;
