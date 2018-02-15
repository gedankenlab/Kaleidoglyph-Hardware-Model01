// -*- c++ -*-

#pragma once

// Basically, this file is here to make the model01::Keyboard class available to
// Kaleidoscope
#include "model01/Keyboard.h"

#include "model01/Key.h"
#include "model01/KeyAddr.h"
#include "model01/LedAddr.h"
#include "model01/Color.h"


namespace kaleidoscope {

// `using` won't work; it will be confined to this block, and have no effect.
typedef model01::Key     Key;
typedef model01::KeyAddr KeyAddr;
typedef model01::LedAddr LedAddr;
typedef model01::Color   Color;

// Maybe hiding this would be better, but here goes for now:
typedef model01::Keyboard Keyboard;

// In order to use it with preprocessor macros, TOTAL_KEYS needs to be a macro
constexpr KeyAddr total_keys = model01::Keyboard::total_keys;
//constexpr KeyAddr TOTAL_KEYS       = model01::Keyboard::total_keys;
//constexpr KeyAddr UNKNOWN_KEY_ADDR = model01::Keyboard::total_keys;

} // namespace kaleidoscope {
