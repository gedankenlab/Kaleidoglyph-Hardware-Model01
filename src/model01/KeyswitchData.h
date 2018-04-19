// -*- c++ -*-

#pragma once

#include <Arduino.h>


namespace kaleidoglyph {
namespace hardware {

// As with "Key", this name should be changed to make it clear what's represented
struct KeyswitchData {
  byte banks[4]; // 32 keys per scanner / 8 bits per byte
};

} // namespace hardware {
} // namespace kaleidoglyph {
