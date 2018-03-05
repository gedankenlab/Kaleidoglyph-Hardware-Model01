// -*- c++ -*-

#pragma once

// Basically, this file is here to make the model01::Keyboard class available to
// Kaleidoscope
//#include "model01/Keyboard.h"

//#include "model01/Key.h"
//#include "model01/KeyAddr.h"
//#include "model01/LedAddr.h"
//#include "model01/Color.h"

// This might be a solution to the faux circular dependency problem
#define KALEIDOSCOPE_KEYADDR_H "model01/KeyAddr.h"
#define KALEIDOSCOPE_LEDADDR_H "model01/LedAddr.h"
#define KALEIDOSCOPE_COLOR_H   "model01/Color.h"

#define KALEIDOSCOPE_HARDWARE_KEYBOARD_H "model01/Keyboard.h"

namespace kaleidoscope {

// `using` won't work; it will be confined to this block, and have no effect.
//typedef model01::Key     Key;
//typedef model01::KeyAddr KeyAddr;
//typedef model01::LedAddr LedAddr;
//typedef model01::Color   Color;

// Maybe hiding this would be better, but here goes for now:
//typedef model01::Keyboard Keyboard;

// In order to use it with preprocessor macros, TOTAL_KEYS needs to be a macro
//constexpr KeyAddr total_keys {model01::total_keys};
//constexpr KeyAddr TOTAL_KEYS       = model01::Keyboard::total_keys;
//constexpr KeyAddr UNKNOWN_KEY_ADDR = model01::Keyboard::total_keys;

} // namespace kaleidoscope {


#define KEYMAP_STACKED(                                                 \
  l_r0c0, l_r0c1, l_r0c2, l_r0c3, l_r0c4, l_r0c5, l_r0c6,               \
  l_r1c0, l_r1c1, l_r1c2, l_r1c3, l_r1c4, l_r1c5,                       \
  l_r2c0, l_r2c1, l_r2c2, l_r2c3, l_r2c4, l_r2c5, l_r2c6,               \
  l_r3c0, l_r3c1, l_r3c2, l_r3c3, l_r3c4, l_r3c5, l_r3c6,               \
  l_t0, l_t1, l_t2, l_t3,                                               \
  l_p0,                                                                 \
                                                                        \
  r_r0c0, r_r0c1, r_r0c2, r_r0c3, r_r0c4, r_r0c5, r_r0c6,               \
          r_r1c1, r_r1c2, r_r1c3, r_r1c4, r_r1c5, r_r1c6,               \
  r_r2c0, r_r2c1, r_r2c2, r_r2c3, r_r2c4, r_r2c5, r_r2c6,               \
  r_r3c0, r_r3c1, r_r3c2, r_r3c3, r_r3c4, r_r3c5, r_r3c6,               \
  r_t0, r_t1, r_t2, r_t3,                                               \
  r_p0)                                                                 \
                                                                        \
  {                                                                     \
    l_t0, l_r0c6, l_r0c5, l_r0c4, l_r0c3, l_r0c2, l_r0c1, l_r0c0,       \
    l_t1, l_r2c6, l_r1c5, l_r1c4, l_r1c3, l_r1c2, l_r1c1, l_r1c0,       \
    l_t2, l_r3c6, l_r2c5, l_r2c4, l_r2c3, l_r2c2, l_r2c1, l_r2c0,       \
    l_t3, l_p0,   l_r3c5, l_r3c4, l_r3c3, l_r3c2, l_r3c1, l_r3c0,       \
                                                                        \
    r_r0c6, r_r0c5, r_r0c4, r_r0c3, r_r0c2, r_r0c1, r_r0c0, r_t3,       \
    r_r1c6, r_r1c5, r_r1c4, r_r1c3, r_r1c2, r_r1c1, r_r2c0, r_t2,       \
    r_r2c6, r_r2c5, r_r2c4, r_r2c3, r_r2c2, r_r2c1, r_r3c0, r_t1,       \
    r_r3c6, r_r3c5, r_r3c4, r_r3c3, r_r3c2, r_r3c1, r_p0,   r_t0,       \
  }
