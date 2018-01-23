// -*- c++ -*-

#pragma once

#include <Arduino.h>


namespace kaleidoscope {

namespace model01 {

// The Model01 has only 64 keys, and we only need one extra value to indicate an invalid
// key address, so that all fits in one byte.
typedef byte KeyAddr;

} // namespace model01 {


namespace keyaddr {

// Everything past this point should be unnecessary, but could possibly be helpful for
// plugins that want to affect whole rows or columns. Note that this code defines 8 rows &
// 8 columns, which is probably not what a user wants on a Model01 (the last four rows are
// on the right half of the keyboard.

// ROWS = 8
// COLS = 8
constexpr KeyAddr ROW_BITS = B00111000;
constexpr KeyAddr COL_BITS = B00000111;

// Conversion functions to help with back-compat and user addressing. I actually think
// that these addresses are not great, since the thumb arc should be a separate row, and
// the palm keys yet another. User addressing should maybe be this 3-tuple: {hand, row,
// col}, but that could be handled with #defines, or the build scripts.
constexpr byte row(KeyAddr key_addr) {
  return (key_addr & ROW_BITS) >> 3;
}

constexpr byte col(KeyAddr key_addr) {
  return (key_addr & COL_BITS);
}

constexpr KeyAddr addr(byte row, byte col) {
  return ((row << 3) | col);
}

} // namespace keyaddr {

} // namespace kaleidoscope {
