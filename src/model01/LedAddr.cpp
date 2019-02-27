// -*- c++ -*-

#include "LedAddr.h"

namespace kaleidoglyph {

namespace hardware {
// Translate from KeyAddr to LedAddr by using this table. The index in the array is the
// KeyAddr; the value is the LedAddr.
static constexpr PROGMEM byte key_led_map[] = {
  27, 26, 20, 19, 12, 11,  4,  3,
  28, 25, 21, 18, 13, 10,  5,  2,
  29, 24, 22, 17, 14,  9,  6,  1,
  30, 31, 23, 16, 15,  8,  7,  0,

  60, 59, 52, 51, 44, 43, 37, 36,
  61, 58, 53, 50, 45, 42, 38, 35,
  62, 57, 54, 49, 46, 41, 39, 34,
  63, 56, 55, 48, 47, 40, 32, 33,
};
} // namespace hardware {

LedAddr::LedAddr(KeyAddr k) {
  addr_ = pgm_read_byte( &(hardware::key_led_map[byte(k)]) );
}

} // namespace kaleidoglyph {
