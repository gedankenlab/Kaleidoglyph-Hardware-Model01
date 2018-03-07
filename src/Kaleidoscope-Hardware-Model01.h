// -*- c++ -*-

#pragma once

// These macros allow other modules to include headers for classes defined in the hardware
// library to be included without triggering any "circular" dependencies.
#define KALEIDOSCOPE_KEYADDR_H "model01/KeyAddr.h"
#define KALEIDOSCOPE_LEDADDR_H "model01/LedAddr.h"
#define KALEIDOSCOPE_COLOR_H   "model01/Color.h"

// This is the main hardware class `kaleidoscope::hardware::Keyboard`. It provides both
// that and widely-used constant `kaleidoscope::hardware::total_keys`:
#define KALEIDOSCOPE_HARDWARE_KEYBOARD_H "model01/Keyboard.h"

// Since the HID module that we need depends on the MCU, we need to define which one to
// include here. This should be a reference to either the HIDAdapter or the HID library
// itself:
#define KALEIDOSCOPE_HID_H "KeyboardioHID.h"


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
