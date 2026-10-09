#pragma once
#include <Arduino.h>
namespace Physics {
String formula(uint8_t target);
String solve(uint8_t target,const String& data);
String autoSolve(const String& data);
const char* name(uint8_t target);
}
