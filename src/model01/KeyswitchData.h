// -*- c++ -*-

#pragma once

#include <Arduino.h>


namespace kaleidoscope {
namespace hardware {

// As with "Key", this name should be changed to make it clear what's represented
union KeyswitchData {
  byte banks[4]; // 32 keys per scanner / 8 bits per byte
  uint32_t all;
};

} // namespace hardware {
} // namespace kaleidoscope {
