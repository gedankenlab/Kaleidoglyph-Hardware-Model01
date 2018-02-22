// -*- c++ -*-

#pragma once

#include <stdint.h>


namespace kaleidoscope {
namespace model01 {

// In the old cRGB struct, the order of the bytes (b,g,r) was important because brace
// initialization was used. for the Color struct, I'm using a real constructor, so that
// won't matter, but I'll leave the order as it was for brace initialization. What really
// matters is the order of the arguments when calling the twi function that sends the
// commands to the LED controller module, and that's handled by the functions that make
// those calls, not this struct.

struct Color {
  uint8_t b;
  uint8_t g;
  uint8_t r;

  Color() = default; // Does not initialize to zeros without `Color c = {};`
  // I'm not really sure why constexpr would help here. I should experiment to see if it
  // produces smaller code. I'm also not sure about the constructor above; this is obtuse
  // C++ magic.
  constexpr Color(uint8_t red, uint8_t green, uint8_t blue)
    : b(blue), g(green), r(red) {}

  // Comparison operators
  bool operator==(const Color& other) const {
    return ((this->b == other.b) &&
            (this->g == other.g) &&
            (this->r == other.r));
  }
  bool operator!=(const Color& other) const {
    return !(*this == other);
  }

};

} // namespace model01 {
} // namespace kaleidoscope {
