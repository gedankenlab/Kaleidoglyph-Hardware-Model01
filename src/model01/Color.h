// -*- c++ -*-

#pragma once

#include <Arduino.h>


namespace kaleidoglyph {

// In the old cRGB struct, the order of the bytes (b,g,r) was important because brace
// initialization was used. for the Color struct, I'm using a real constructor, so that
// won't matter, but I'll leave the order as it was for brace initialization. What really
// matters is the order of the arguments when calling the twi function that sends the
// commands to the LED controller module, and that's handled by the functions that make
// those calls, not this struct.

// TODO: consider changing this two a two-byte struct as a bitfield with 5 bits per
// color. This would save space at the cost of speed. Would need to scale up to 8 bits by
// multiplying by 8 (three-bit shift). To change that, it would make sense to change from
// struct to class, and make the data members private.

class Color {

 private:
  byte r_ : 5, g_ : 5, b_ : 5;

 public:
  // Public interface functions, using 8-bit values. The secondary two-bit shift is used
  // so that we get the full range from zero to 255, even though the values are only 5
  // bits internally.
  byte red  () const { return (r_ << 3) | (r_ >> 2); }
  byte green() const { return (g_ << 3) | (g_ >> 2); }
  byte blue () const { return (b_ << 3) | (g_ >> 2); }

  void red  (byte r8) { r_ = r8 >> 3; }
  void green(byte g8) { g_ = g8 >> 3; }
  void blue (byte b8) { b_ = b8 >> 3; }

  // This is tricky -- these functions shouldn't be used outside the hardware module, so
  // they should probably be private, accessible to the Scanner class as a friend. These
  // access the 5-bit color values directly.
  byte r() const { return r_; }
  byte g() const { return g_; }
  byte b() const { return b_; }

  void r(byte r5) { r_ = r5; }
  void g(byte g5) { g_ = g5; }
  void b(byte b5) { b_ = b5; }

  Color() = default;

  constexpr
  Color(byte r8, byte g8, byte b8) : r_(r8 >> 3),
                                     g_(g8 >> 3),
                                     b_(b8 >> 3)  {}

  constexpr
  Color(uint16_t raw) : r_(raw           ),
                        g_(raw >>  5     ),
                        b_(raw >> (5 + 5))  {}

  bool operator!=(const Color& other) const {
    return ((this->r_ != other.r_) ||
            (this->g_ != other.g_) ||
            (this->b_ != other.b_)   );
  }
  bool operator==(const Color& other) const {
    return !(*this != other);
  }
};

} // namespace kaleidoglyph {
