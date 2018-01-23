// -*- c++ -*-

#pragma once

#include <Arduino.h>


namespace kaleidoscope {
namespace model01 {

// I think we should also have a type for LED index. Warning: this is tricky, because
// "KeyAddr" & "LedAddr" are actually the same type, so we can't use two functions, one of
// which takes a KeyAddr, and a different one that takes LedAddr as a param. The compiler
// will catch it, though.
typedef byte LedAddr;

} // namespace model01 {
} // namespace kaleidoscope {
