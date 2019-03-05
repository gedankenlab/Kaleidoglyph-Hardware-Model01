// -*- c++ -*-

#include <Arduino.h>
#include "model01/KeyAddr.h"

namespace kaleidoglyph {

namespace hardware {

constexpr byte total_rows =  6;
constexpr byte total_cols = 18;

constexpr byte pos(byte r, byte c) {
  return (r * total_cols + c);
}
constexpr byte row(byte pos) {
  return pos / total_cols;
}
constexpr byte col(byte pos) {
  return pos % total_cols;
}

// Translate from KeyAddr to encoded row & col
static constexpr PROGMEM byte key_pos_map[] = {
  pos(4, 5), pos(0, 6), pos(0, 5), pos(0, 4), pos(0, 3), pos(0, 2), pos(0, 1), pos(0, 0),
  pos(4, 6), pos(2, 6), pos(1, 5), pos(1, 4), pos(1, 3), pos(1, 2), pos(1, 1), pos(1, 0),
  pos(4, 7), pos(3, 6), pos(2, 5), pos(2, 4), pos(2, 3), pos(2, 2), pos(2, 1), pos(2, 0),
  pos(4, 8), pos(5, 6), pos(3, 5), pos(3, 4), pos(3, 3), pos(3, 2), pos(3, 1), pos(3, 0),

  pos(0,17), pos(0,16), pos(0,15), pos(0,14), pos(0,13), pos(0,12), pos(0,11), pos(4,12),
  pos(1,17), pos(1,16), pos(1,15), pos(1,14), pos(1,13), pos(1,12), pos(2,11), pos(4,11),
  pos(2,17), pos(2,16), pos(2,15), pos(2,14), pos(2,13), pos(2,12), pos(3,11), pos(4,10),
  pos(3,17), pos(3,16), pos(3,15), pos(3,14), pos(3,13), pos(3,12), pos(5,11), pos(4, 9),
};
} // namespace hardware {

byte KeyAddr::lookupCoords(KeyAddr k) {
  byte coord = pgm_read_byte( &(hardware::key_pos_map[byte(k)]) );
}

byte KeyAddr::lookupAddr(byte row, byte col) {
  if (row < 4) {
    if (col < 6) {
      return (row * 8) + (7 - col);
    }
    if (col > 11) {
      return ((row + 4) * 8) + (17 - col);
    }
  }
  byte r;
  byte c;
  switch (row * total_cols + col) {
    case pos(4,5):
      return 0 * 8 + 0;
    case pos(4,6):
      return 1 * 8 + 0;
    case pos(4,7):
      return 2 * 8 + 0;
    case pos(4,8):
      return 3 * 8 + 0;
    case pos(0,6):
      return 0 * 8 + 1;
    case pos(2,6):
      return 1 * 8 + 1;
    case pos(3,6):
      return 2 * 8 + 1;
    case pos(5,6):
      return 3 * 8 + 1;
    case pos(0,11):
      return 4 * 8 + 6;
    case pos(2,11):
      return 5 * 8 + 6;
    case pos(3,11):
      return 6 * 8 + 6;
    case pos(5,11):
      return 7 * 8 + 6;
    case pos(4,12):
      return 4 * 8 + 7;
    case pos(4,11):
      return 5 * 8 + 7;
    case pos(4,10):
      return 6 * 8 + 7;
    case pos(4, 9):
      return 7 * 8 + 7;
  }
  return 64;
}

} // namespace kaleidoglyph {
