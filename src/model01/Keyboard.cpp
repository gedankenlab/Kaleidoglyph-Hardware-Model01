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
  memcpy(&prev_scan_, &curr_scan_, sizeof(prev_scan_));

  // scan left hand
  scanners_[0].readKeys(curr_scan_.hands[0]);

  // scan right hand
  scanners_[1].readKeys(curr_scan_.hands[1]);

  // backcompat
  //actOnMatrixScan();
}

// get the address of the next key that changed state (if any), starting from the last one
// checked (or at least, that's the expected usage). Return true if we found an event;
// false if we didn't find a keyswitch in a different state. Maybe it should return the
// state value instead.
KeyswitchEvent Keyboard::nextKeyswitchEvent(KeyAddr& k) {
  // compare state bitfield one byte at a time until we find one that differs
  for (byte r = (k.addr / 8); r < 8; ++r) {
    if (curr_scan_.banks[r] == prev_scan_.banks[r]) {
      continue;
    }
    // next compare the bits one at a time
    for (byte c = (k.addr % 8); c < 8; ++c) {
      byte prev_state = bitRead(prev_scan_.banks[r], c);
      byte curr_state = bitRead(curr_scan_.banks[r], c);
      if (prev_state != curr_state) {
        k.addr = (r << 3) | c;
        return {Key_NoKey, k, KeyswitchState(curr_state, prev_state)};
      }
    }
  }
  // key_addr = Keyboard::total_keys;
  return {Key_NoKey, k, KeyswitchState(0)};
}

#if 0
bool Keyboard::Iterator::operator!=(const Iterator& other) {

  byte r = addr_ / 8;
  byte end = other.addr_ / 8;

  while (r < end) {
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
      addr_ = (r * 8) + c;
    } else {
      ++r;
      c = 0;
    }
  }
  return false;



  // First, the actual end condition test (maybe better to use < instead of !=):
  while (addr_ < other.addr_) {
    // Compare the next bank (row). If it hasn't changed, skip ahead to the next one:
    if (keyboard_.curr_scan_.banks[row_] != keyboard_.prev_scan_.banks[row_]) {
      // We found a bank that changed. Test the current bit:
      do {
        byte curr_state = bitRead(keyboard_.curr_scan_.banks[row_], col_);
        byte prev_state = bitRead(keyboard_.prev_scan_.banks[row_], col_);
        if (curr_state != prev_state) {
          // We found a bit that changed; get the event ready and return true:
          event.state = KeyswitchState(curr_state, prev_state);
          event.addr = KeyAddr(addr_);
          event.key = Key_Transparent;
          return true;
        } else {
          // If this bit hasn't changed, check the next one:
          ++addr_; // Incrementing addr_ also increments c (which overflows to r)
          continue; // superfluous (goes to the bottom of do/while c != 0)
        }
        // If c == 0, that means we've reached the end of the current bank, and can go
        // on to the next one (starting back at the beginning of the enclosing loop)
      } while (col_ != 0);
      continue; // this is superfluous, but maybe clearer?
    } else {
      // no keyswitch states changed in this bank; advance to the next one:
      ++row_;
      col_ = 0;
      continue; // also superfluous, but maybe clearer?
    }
  } // while (addr_ != other.addr_) {
  return false;
}
#endif

// return the state of the keyswitch as a bitfield
KeyswitchState Keyboard::keyswitchState(KeyAddr k) const {
  byte state = 0;
  byte r = k.addr / 8;
  byte c = k.addr % 8;
  return KeyswitchState(bitRead(curr_scan_.banks[r], c),
                        bitRead(prev_scan_.banks[r], c));
}


constexpr byte HAND_BIT = B00100000;

LedAddr Keyboard::getLedAddr(KeyAddr k) const {
  return LedAddr(pgm_read_byte(&(key_led_map[k.addr])));
}

Color Keyboard::getLedColor(LedAddr led) const {
  bool hand = led.addr & HAND_BIT; // B00100000
  return scanners_[hand].getLedColor(led.addr & ~HAND_BIT);
}

void Keyboard::setLedColor(LedAddr led, Color color) {
  bool hand = led.addr & HAND_BIT; // B00100000
  scanners_[hand].setLedColor(led.addr & ~HAND_BIT, color);
}

Color Keyboard::getKeyColor(KeyAddr k) const {
  LedAddr led = getLedAddr(k);
  return getLedColor(led);
}

void Keyboard::setKeyColor(KeyAddr k, Color color) {
  LedAddr led = getLedAddr(k);
  setLedColor(led, color);
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
  memset(&curr_scan_, 0, sizeof(curr_scan_));
  memset(&prev_scan_, 0, sizeof(prev_scan_));

  TWBR = 12; // This is 400mhz, which is the fastest we can drive the ATTiny

  // Turn off all LEDs at startup. TODO: move this elsewhere?
  scanners_[0].updateAllLeds(Color{0,0,0});
  scanners_[1].updateAllLeds(Color{0,0,0});
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

// backcompat
kaleidoscope::hardware::Keyboard KeyboardHardware;
