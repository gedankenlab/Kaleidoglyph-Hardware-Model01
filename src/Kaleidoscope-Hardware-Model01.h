// -*- c++ -*-

#pragma once

#include <Arduino.h>

// Basically, this file is here to make the model01::Keyboard class available to
// Kaleidoscope
#include "model01/Keyboard.h"

//#include "model01/Key.h"
#include "model01/KeyAddr.h"
#include "model01/LedAddr.h"
#include "model01/Color.h"


namespace kaleidoscope {

// `using` won't work; it will be confined to this block, and have no effect.
//typedef model01::Key     Key;
typedef model01::KeyAddr KeyAddr;
typedef model01::LedAddr LedAddr;
typedef model01::Color   Color;

// Maybe hiding this would be better, but here goes for now:
//typedef model01::Keyboard Keyboard;

// In order to use it with preprocessor macros, TOTAL_KEYS needs to be a macro
constexpr byte total_keys { model01::total_keys };
//constexpr KeyAddr TOTAL_KEYS       = model01::Keyboard::total_keys;
//constexpr KeyAddr UNKNOWN_KEY_ADDR = model01::Keyboard::total_keys;

} // namespace kaleidoscope {


#define KEYMAP_STACKED(                                                 \
               r0c0, r0c1, r0c2, r0c3, r0c4, r0c5, r0c6,                \
               r1c0, r1c1, r1c2, r1c3, r1c4, r1c5, r1c6,                \
               r2c0, r2c1, r2c2, r2c3, r2c4, r2c5,                      \
               r3c0, r3c1, r3c2, r3c3, r3c4, r3c5, r2c6,                \
               r0c7, r1c7, r2c7, r3c7,                                  \
               r3c6,                                                    \
                                                                        \
               r0c9,  r0c10, r0c11, r0c12, r0c13, r0c14, r0c15,         \
               r1c9,  r1c10, r1c11, r1c12, r1c13, r1c14, r1c15,         \
                      r2c10, r2c11, r2c12, r2c13, r2c14, r2c15,         \
               r2c9,  r3c10, r3c11, r3c12, r3c13, r3c14, r3c15,         \
               r3c8,  r2c8,  r1c8, r0c8,                                \
               r3c9)                                                    \
  {                                                                     \
    {r0c0, r0c1, r0c2, r0c3, r0c4, r0c5, r0c6, r0c7, r0c8, r0c9, r0c10, r0c11, r0c12, r0c13, r0c14, r0c15}, \
    {r1c0, r1c1, r1c2, r1c3, r1c4, r1c5, r1c6, r1c7, r1c8, r1c9, r1c10, r1c11, r1c12, r1c13, r1c14, r1c15}, \
    {r2c0, r2c1, r2c2, r2c3, r2c4, r2c5, r2c6, r2c7, r2c8, r2c9, r2c10, r2c11, r2c12, r2c13, r2c14, r2c15}, \
    {r3c0, r3c1, r3c2, r3c3, r3c4, r3c5, r3c6, r3c7, r3c8, r3c9, r3c10, r3c11, r3c12, r3c13, r3c14, r3c15}, \
  }
