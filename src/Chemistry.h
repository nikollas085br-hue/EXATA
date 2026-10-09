#pragma once
#include <Arduino.h>
namespace Chemistry {
struct Element { const char* symbol; const char* name; uint8_t z; const char* group; const char* period; const char* category; const char* config; };
const Element* byAtomicNumber(uint8_t z);
const Element* find(const String& query);
String bondType(const Element& a,const Element& b);
String octetNote(const Element& e);
const Element* all();
size_t count();
}
