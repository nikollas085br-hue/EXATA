// EXATA SD layer: simple text-oriented library for study materials.
#include "SDManager.h"
#include <SD.h>
#include <SPI.h>

namespace SDManager {
static bool mounted = false;
static const char* folders[] = {
    "/EXATA_SD", "/EXATA_SD/MATEMATICA", "/EXATA_SD/FISICA",
    "/EXATA_SD/QUIMICA", "/EXATA_SD/BIOLOGIA", "/EXATA_SD/NOTAS",
    "/EXATA_SD/FORMULAS", "/EXATA_SD/CARTOES", "/EXATA_SD/MODULOS",
    "/EXATA_SD/DIAGRAMAS"
};

bool begin() {
    SPI.begin(40, 39, 14, 12);
    mounted = SD.begin(12, SPI, 25000000);
    if (!mounted) return false;
    for (size_t i=0; i<sizeof(folders)/sizeof(folders[0]); ++i) SD.mkdir(folders[i]);
    return true;
}
bool ready() { return mounted; }
bool isDir(const String& path) {
    if (!mounted) return false;
    File f=SD.open(path); bool result=f && f.isDirectory(); if(f)f.close(); return result;
}
bool exists(const String& path) {
    if (!mounted) return false;
    File f=SD.open(path); bool result=bool(f); if(f)f.close(); return result;
}
String normalize(const String& base, const String& name) {
    String result;
    if (name.startsWith("/")) result=name;
    else if (base.endsWith("/")) result=base+name;
    else result=base+"/"+name;
    while (result.indexOf("//")>=0) result.replace("//","/");
    if (!result.startsWith("/EXATA_SD")) return "/EXATA_SD";
    if (result.indexOf("..")>=0) return "/EXATA_SD";
    return result;
}
String list(const String& path) {
    if (!mounted) return "SD nao disponivel";
    String safe=normalize("/EXATA_SD",path);
    File dir=SD.open(safe);
    if (!dir || !dir.isDirectory()) { if(dir)dir.close(); return "Pasta invalida"; }
    String out;
    File f;
    while ((f=dir.openNextFile())) {
        String n=String(f.name()); int slash=n.lastIndexOf('/'); if(slash>=0)n=n.substring(slash+1);
        out += f.isDirectory() ? "[D] " : "[F] "; out += n; out += '\n'; f.close();
    }
    dir.close();
    if (!out.length()) out="(vazio)";
    return out;
}
String readText(const String& path) {
    if (!mounted) return "SD nao disponivel";
    String safe=normalize("/EXATA_SD",path); File f=SD.open(safe,"r");
    if (!f || f.isDirectory()) { if(f)f.close(); return "Nao foi possivel abrir o arquivo."; }
    String out; while(f.available() && out.length()<12000) out+=(char)f.read(); f.close(); return out;
}
bool writeText(const String& path, const String& data) {
    if (!mounted) return false;
    String safe=normalize("/EXATA_SD",path); int slash=safe.lastIndexOf('/');
    if(slash>0) { String dir=safe.substring(0,slash); if(!SD.exists(dir)) SD.mkdir(dir); }
    File f=SD.open(safe,"w"); if(!f)return false; size_t n=f.print(data); f.close(); return n==data.length();
}
bool renameFile(const String& from, const String& to) {
    if (!mounted) return false;
    String a=normalize("/EXATA_SD",from), b=normalize("/EXATA_SD",to);
    if(a==b || !SD.exists(a) || SD.exists(b)) return false;
    int ea=a.lastIndexOf('.'), eb=b.lastIndexOf('.'); String xa=ea>=0?a.substring(ea):"", xb=eb>=0?b.substring(eb):""; xa.toLowerCase(); xb.toLowerCase();
    if(xa!=xb) return false;
    return SD.rename(a,b);
}
}
