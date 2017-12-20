#include <Kaleidoscope.h>
#include <KeyboardioHID.h>
#include <avr/wdt.h>

namespace kaleidoscope {

KeyboardioScanner Model01::leftHand(0);
KeyboardioScanner Model01::rightHand(3);
bool Model01::isLEDChanged = true;
keydata_t Model01::leftHandMask;
keydata_t Model01::rightHandMask;

// *INDENT-OFF*
static constexpr uint8_t key_led_map[TOTAL_KEYS] = {
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

Model01::Model01(void) {}

void Model01::enableScannerPower(void) {
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
void Model01::enableHighPowerLeds(void) {
  // PE6
  //    pinMode(7, OUTPUT);
  //    digitalWrite(7, LOW);
  DDRE |= _BV(6);
  PORTE &= ~_BV(6);

  // Set B4, the overcurrent check to an input with an internal pull-up
  DDRB &= ~_BV(4);	// set bit, input
  PORTB &= ~_BV(4);	// set bit, enable pull-up resistor
}

void Model01::setup(void) {
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


void Model01::setCrgbAt(KeyAddr key_addr, cRGB crgb) {
  byte i = key_led_map[key_addr];
  if (i < 32) {
    cRGB oldColor = getCrgbAt(i);
    isLEDChanged |= !(oldColor.r == crgb.r && oldColor.g == crgb.g && oldColor.b == crgb.b);

    leftHand.ledData.leds[i] = crgb;
  } else if (i < 64) {
    cRGB oldColor = getCrgbAt(i);
    isLEDChanged |= !(oldColor.r == crgb.r && oldColor.g == crgb.g && oldColor.b == crgb.b);

    rightHand.ledData.leds[i - 32] = crgb;
  } else {
    // TODO(anyone):
    // how do we want to handle debugging assertions about crazy user
    // code that would overwrite other memory?
  }
}

uint8_t Model01::getLedIndex(KeyAddr key_addr) {
  return key_led_map[key_addr];
}

cRGB Model01::getCrgbAt(KeyAddr key_addr) {
  uint8_t i = key_led_map[key_addr];
  if (i < 32) {
    return leftHand.ledData.leds[i];
  } else if (i < 64) {
    return rightHand.ledData.leds[i - 32] ;
  } else {
    return {0, 0, 0};
  }
}

void Model01::syncLeds() {
  if (!isLEDChanged)
    return;

  leftHand.sendLEDData();
  rightHand.sendLEDData();

  leftHand.sendLEDData();
  rightHand.sendLEDData();

  leftHand.sendLEDData();
  rightHand.sendLEDData();

  leftHand.sendLEDData();
  rightHand.sendLEDData();

  isLEDChanged = false;
}

boolean Model01::ledPowerFault() {
  if (PINB & _BV(4)) {
    return true;
  } else {
    return false;
  }
}

void debugKeyswitchEvent(keydata_t state, keydata_t previousState, KeyAddr key_addr) {
  byte row = kaleidoscope::keyaddr::row(key_addr);
  byte col = kaleidoscope::keyaddr::col(key_addr);
  if (bitRead(state.all, key_addr) != bitRead(previousState.all, key_addr)) {
    Serial.print("Looking at row ");
    Serial.print(row);
    Serial.print(", col ");
    Serial.print(col);
    Serial.print(" key # ");
    Serial.print(key_addr);
    Serial.print(" ");
    Serial.print(bitRead(previousState.all, key_addr));
    Serial.print(" -> ");
    Serial.print(bitRead(state.all, key_addr));
    Serial.println();
  }
}


void Model01::readMatrix() {
  //scan the Keyboard matrix looking for connections
  previousLeftHandState = leftHandState;
  previousRightHandState = rightHandState;

  if (leftHand.readKeys()) {
    leftHandState = leftHand.getKeyData();
  }

  if (rightHand.readKeys()) {
    rightHandState = rightHand.getKeyData();
  }
}

// tell if any keyswitch was pressed or released
bool Model01::stateChanged() {
  if (leftHandState.all != previousLeftHandState.all ||
      rightHandState.all != previousRightHandState.all)
    return true;
  return false;
}

// get the address of the next key that changed state (if any)
KeyAddr Model01::getNextKeyswitchEvent(KeyAddr key_addr) {
  for (byte r = keyaddr::row(key_addr); r < ROWS; ++r) {
    if (keyboard_state_[r] == prev_keyboard_state_[r])
      continue;
    for (byte c = keyaddr::col(key_addr); c < COLS; ++c) {
      if (bitRead(keyboard_state_[r], c) != bitRead(prev_keyboard_state_[r], c))
	return keyaddr::addr(r, c);
    }
  }
  return UNKNOWN_KEY_ADDR;
}


// probably also soon obsolete
void Model01::actOnMatrixScan() {
  for (KeyAddr key_addr = 0; key_addr < (TOTAL_KEYS/2); key_addr++) {

    uint8_t keyState = (bitRead(previousLeftHandState.all, key_addr) << 0) |
                       (bitRead(leftHandState.all, key_addr) << 1);
    handleKeyswitchEvent(Key_NoKey, key_addr, keyState);

    keyState = (bitRead(previousRightHandState.all, key_addr) << 0) |
               (bitRead(rightHandState.all, key_addr) << 1);

    handleKeyswitchEvent(Key_NoKey, (key_addr | KEYADDR_HAND_BIT), keyState);
  }
}


void Model01::scanMatrix() {
  readMatrix();
  actOnMatrixScan();
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

// These masking fuctions should become obsolete soon
namespace keymask {
constexpr KeyAddr HAND_BIT = B00100000;
constexpr KeyAddr ROW_BITS = B00011000;
constexpr KeyAddr COL_BITS = B00000111;
constexpr KeyAddr mask_bit(KeyAddr key_addr) {
  return (B10000000 >> (key_addr & COL_BITS));
}
} // namespace keymask {

void Model01::maskKey(KeyAddr key_addr) {
  if (key_addr >= TOTAL_KEYS)
    return;

  if (key_addr & keymask::HAND_BIT) {
    rightHandMask.rows[key_addr & keymask::ROW_BITS] |= keymask::mask_bit(key_addr);
  } else {
    leftHandMask.rows[key_addr & keymask::ROW_BITS] |= keymask::mask_bit(key_addr);
  }
}

void Model01::unMaskKey(KeyAddr key_addr) {
  if (key_addr >= TOTAL_KEYS)
    return;

  if (key_addr & keymask::HAND_BIT) {
    rightHandMask.rows[key_addr & keymask::ROW_BITS] &= ~keymask::mask_bit(key_addr);
  } else {
    leftHandMask.rows[key_addr & keymask::ROW_BITS] &= ~keymask::mask_bit(key_addr);
  }
}

bool Model01::isKeyMasked(KeyAddr key_addr) {
  if (key_addr >= TOTAL_KEYS)
    return false;

  if (key_addr & keymask::HAND_BIT) {
    return rightHandMask.rows[key_addr & keymask::ROW_BITS] & keymask::mask_bit(key_addr);
  } else {
    return leftHandMask.rows[key_addr & keymask::ROW_BITS] & keymask::mask_bit(key_addr);
  }
}

void Model01::maskHeldKeys(void) {
  memcpy(leftHandMask.rows, leftHandState.rows, sizeof(leftHandMask));
  memcpy(rightHandMask.rows, rightHandState.rows, sizeof(rightHandMask));
}


void Model01::setKeyscanInterval(uint8_t interval) {
  leftHand.setKeyscanInterval(interval);
  rightHand.setKeyscanInterval(interval);
}

} // namespace kaleidoscope {

HARDWARE_IMPLEMENTATION KeyboardHardware;
