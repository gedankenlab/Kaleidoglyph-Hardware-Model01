// Why don't we #include "Model01.h"? I guess we get it via <Kaleidoscope.h>
#include <Kaleidoscope.h>
// Does HID interface stuff really belong here?
#include <KeyboardioHID.h>
// todo: look up wdt.h
#include <avr/wdt.h>

namespace kaleidoscope {
namespace hardware {

// Why don't we do these things in the constructor? Why are they static? There's only one object...
KeyboardioScanner Model01::leftHand(0);
KeyboardioScanner Model01::rightHand(3);
bool Model01::isLEDChanged = true;

// This needs rearranging because of KeyAddr
static constexpr uint8_t key_led_map[4][16] = {
  {3, 4, 11, 12, 19, 20, 26, 27,     36, 37, 43, 44, 51, 52, 59, 60},
  {2, 5, 10, 13, 18, 21, 25, 28,     35, 38, 42, 45, 50, 53, 58, 61},
  {1, 6, 9, 14, 17, 22, 24, 29,     34, 39, 41, 46, 49, 54, 57, 62},
  {0, 7, 8, 15, 16, 23, 31, 30,     33, 32, 40, 47, 48, 55, 56, 63},
};



void Model01::enableScannerPower() {
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
void Model01::enableHighPowerLeds() {
  // PE6
  //    pinMode(7, OUTPUT);
  //    digitalWrite(7, LOW);
  DDRE |= _BV(6);
  PORTE &= ~_BV(6);

  // Set B4, the overcurrent check to an input with an internal pull-up
  DDRB &= ~_BV(4);	// set bit, input
  PORTB &= ~_BV(4);	// set bit, enable pull-up resistor
}

void Model01::setup() {
  wdt_disable();
  delay(100);
  enableScannerPower();

  // Consider not doing this until 30s after keyboard
  // boot up, to make it easier to rescue things
  // in case of power draw issues.
  enableHighPowerLeds();
  leftHandState.all = 0;
  rightHandState.all = 0;

  TWBR = 12; // This is 400mhz, which is the fastest we can drive the ATTiny
}


LedAddr Model01::getLedAddr(KeyAddr key_addr) {
  return key_led_map[key_addr];
}

Color Model01::getLedColor(LedAddr led_addr) {
  bool hand = led_addr & HAND_BIT; // B10000000
  return scanners_[hand].getLedColor(led_addr & ~HAND_BIT);
}

void Model01::setLedColor(LedAddr led_addr, Color color) {
  bool hand = led_addr & HAND_BIT; // B10000000
  scanners_[hand].setLedColor(led_addr & ~HAND_BIT, color);
}

Color Model01::getKeyColor(KeyAddr key_addr) {
  LedAddr led_addr = getLedAddr(key_addr);
  return getLedColor(led_addr);
}

void Model01::setKeyColor(KeyAddr key_addr, Color color) {
  LedAddr led_addr = getLedAddr(key_addr);
  setLedColor(led_addr, color);
}


// This function is a bit better now, but I still feel the desire to write this as an
// explicit loop
void Model01::updateLeds() {
  scanners_[0].updateNextLedBank();
  scanners_[1].updateNextLedBank();

  scanners_[0].updateNextLedBank();
  scanners_[1].updateNextLedBank();

  scanners_[0].updateNextLedBank();
  scanners_[1].updateNextLedBank();

  scanners_[0].updateNextLedBank();
  scanners_[1].updateNextLedBank();
}


boolean Model01::ledPowerFault() {
  if (PINB & _BV(4)) {
    return true;
  } else {
    return false;
  }
}

void debugKeyswitchEvent(keydata_t state, keydata_t previousState, uint8_t keynum, uint8_t row, uint8_t col) {
  if (bitRead(state.all, keynum) != bitRead(previousState.all, keynum)) {
    Serial.print("Looking at row ");
    Serial.print(row);
    Serial.print(", col ");
    Serial.print(col);
    Serial.print(" key # ");
    Serial.print(keynum);
    Serial.print(" ");
    Serial.print(bitRead(previousState.all, keynum));
    Serial.print(" -> ");
    Serial.print(bitRead(state.all, keynum));
    Serial.println();
  }
}


void Model01::scanMatrix() {
  // copy current keyswitch state array to previous
  memcpy(prev_state_, curr_state_, sizeof(prev_state_));

  // scan left hand
  if (scanners_[0].readKeys())
    curr_state_[0] = scanners_[0].getKeyData();
  // scan right hand
  if (scanners_[1].readKeys())
    curr_state_[1] = scanners_[1].getKeyData();
}


void Model01::rebootBootloader() {
  // Set the magic bits to get a Caterina-based device
  // to reboot into the bootloader and stay there, rather
  // than run move onward
  //
  // These values are the same as those defined in
  // Caterina.c

  uint16_t bootKey = 0x7777;
  uint16_t *const bootKeyPtr = reinterpret_cast<uint16_t *>(0x0800);

  // Stash the magic key
  *bootKeyPtr = bootKey;

  // Set a watchdog timer
  wdt_enable(WDTO_120MS);

  while (1) {} // This infinite loop ensures nothing else
  // happens before the watchdog reboots us
}


void Model01::setKeyscanInterval(uint8_t interval) {
  leftHand.setKeyscanInterval(interval);
  rightHand.setKeyscanInterval(interval);
}

// Why do we declare this object here?
HARDWARE_IMPLEMENTATION KeyboardHardware;

} // namespace hardware {
} // namespace kaleidoscope {
