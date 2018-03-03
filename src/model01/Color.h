// -*- c++ -*-

#pragma once

#include <Arduino.h>


namespace kaleidoscope {

// In the old cRGB struct, the order of the bytes (b,g,r) was important because brace
// initialization was used. for the Color struct, I'm using a real constructor, so that
// won't matter, but I'll leave the order as it was for brace initialization. What really
// matters is the order of the arguments when calling the twi function that sends the
// commands to the LED controller module, and that's handled by the functions that make
// those calls, not this struct.

struct Color {

 public:
  // Don't give these member variables default values, or Scanner.cpp will have a
  // compilation error in the `led_states_` union.
  byte b;
  byte g;
  byte r;

  Color() = default; // Does not initialize to zeros without `Color c = {};`
  // I'm not really sure why constexpr would help here. I should experiment to see if it
  // produces smaller code. I'm also not sure about the constructor above; this is obtuse
  // C++ magic.
  constexpr Color(byte red, byte green, byte blue)
    : b(blue), g(green), r(red) {}

  // Comparison operators
  bool operator==(Color const &other) const {
    return ((this->b == other.b) &&
            (this->g == other.g) &&
            (this->r == other.r));
  }
  bool operator!=(Color const &other) const {
    return !(*this == other);
  }

  // TODO: add math operators? They could be useful to some plugin or other

};

} // namespace kaleidoscope {
