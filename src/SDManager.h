#pragma once
#include <Arduino.h>
namespace SDManager {
bool begin(); bool ready(); bool isDir(const String& path);
String normalize(const String& base,const String& name);
String list(const String& path);
String readText(const String& path);
bool writeText(const String& path,const String& data);
bool renameFile(const String& from,const String& to);
bool exists(const String& path);
}
