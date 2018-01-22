// -*- c++ -*-

#pragma once

namespace kaleidoscope {
namespace hardware {

// As with "Key", this name should be changed to make it clear what's represented
union KeyData {
  byte rows[4];
  uint32_t all;
};

} // namespace hardware {
} // namespace kaleidoscope {
