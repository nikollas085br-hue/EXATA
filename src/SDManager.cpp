#include "SDManager.h"
#include <SD.h>
#include <SPI.h>
namespace SDManager {
static bool ok=false;
bool begin(){
  SPI.begin(40,39,14,12);
  ok=SD.begin(12,SPI,25000000);
  if(ok){
    SD.mkdir("/EXATA_SD");
    SD.mkdir("/EXATA_SD/MATEMATICA");
    SD.mkdir("/EXATA_SD/FISICA");
    SD.mkdir("/EXATA_SD/QUIMICA");
    SD.mkdir("/EXATA_SD/BIOLOGIA");
    SD.mkdir("/EXATA_SD/NOTAS");
    SD.mkdir("/EXATA_SD/FORMULAS");
    SD.mkdir("/EXATA_SD/CARTOES");
    SD.mkdir("/EXATA_SD/MODULOS");
    SD.mkdir("/EXATA_SD/IMAGENS");
    SD.mkdir("/EXATA_SD/DIAGRAMAS");
  }
  return ok;
}
bool ready(){return ok;}
bool isDir(const String&p){if(!ok)return false;File f=SD.open(p);bool r=f&&f.isDirectory();if(f)f.close();return r;}
bool exists(const String&p){if(!ok)return false;File f=SD.open(p);bool r=!!f;if(f)f.close();return r;}
String normalize(const String&b,const String&n){
  if(n.indexOf("..")>=0)return "/EXATA_SD";
  if(n.startsWith("/"))return n;
  if(b=="/")return "/EXATA_SD/"+n;
  return b+"/"+n;
}
String list(const String&path){if(!ok)return "SD nao disponivel";File dir=SD.open(path);if(!dir||!dir.isDirectory()){if(dir)dir.close();return "Pasta invalida";}String out="";File f;while((f=dir.openNextFile())){out+=f.isDirectory()?"[D] ":"[F] ";String n=String(f.name());int slash=n.lastIndexOf('/');if(slash>=0)n=n.substring(slash+1);out+=n;out+='\n';f.close();}dir.close();if(!out.length())out="(vazio)";return out;}
String readText(const String&p){if(!ok)return "SD nao disponivel";File f=SD.open(p,"r");if(!f)return "Nao foi possivel abrir.";String out="";while(f.available()&&out.length()<12000)out+=(char)f.read();f.close();return out;}
bool writeText(const String&p,const String&d){if(!ok)return false;int slash=p.lastIndexOf('/');if(slash>0){String dir=p.substring(0,slash);if(!SD.exists(dir))SD.mkdir(dir);}File f=SD.open(p,"w");if(!f)return false;f.print(d);f.close();return true;}
bool renameFile(const String&from,const String&to){if(!ok||from==to||!SD.exists(from)||SD.exists(to))return false;int a=from.lastIndexOf('.'),b=to.lastIndexOf('.');String ea=a>=0?from.substring(a):"", eb=b>=0?to.substring(b):"";ea.toLowerCase();eb.toLowerCase();if(ea!=eb)return false;return SD.rename(from,to);}
}
