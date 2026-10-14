#include <M5Cardputer.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <SPI.h>
#include <SD.h>
#include <FS.h>

// HOTSPOT ADV UNIVERSAL — servidor local de sites selecionáveis no microSD.
// SD SPI pinout baseado no projeto de referência do Cardputer: SCK=40, MISO=39, MOSI=14, CS=12.
static SPIClass sdSPI(FSPI);
static bool sdOK = false;
static bool apRunning = false;
static WebServer server(80);
static DNSServer dns;
static IPAddress apIP(192,168,4,1);
static char ssid[33] = "HOTSPOT-ADV";
static char password[64] = "12345678";
static String selectedSite = "/sites/exemplo";
static String statusLine = "Iniciando...";
static int menuIndex = 0;
static const char* menuItems[] = {"Iniciar / parar hotspot", "Escolher site do SD", "Configurar nome da rede", "Configurar senha", "Dispositivos conectados", "Estado do cartao SD"};
static const int MENU_COUNT = 6;

String trimPath(String p) {
  p.replace("\\", "/");
  while (p.indexOf("..") >= 0) p.replace("..", "");
  if (!p.startsWith("/")) p = "/" + p;
  return p;
}
String mimeType(String path) {
  path.toLowerCase();
  if (path.endsWith(".html") || path.endsWith(".htm")) return "text/html; charset=utf-8";
  if (path.endsWith(".css")) return "text/css; charset=utf-8";
  if (path.endsWith(".js")) return "application/javascript; charset=utf-8";
  if (path.endsWith(".json")) return "application/json; charset=utf-8";
  if (path.endsWith(".svg")) return "image/svg+xml";
  if (path.endsWith(".png")) return "image/png";
  if (path.endsWith(".jpg") || path.endsWith(".jpeg")) return "image/jpeg";
  if (path.endsWith(".gif")) return "image/gif";
  if (path.endsWith(".ico")) return "image/x-icon";
  if (path.endsWith(".txt")) return "text/plain; charset=utf-8";
  if (path.endsWith(".wasm")) return "application/wasm";
  return "application/octet-stream";
}
void drawScreen() {
  auto &d = M5Cardputer.Display;
  d.fillScreen(0x0000);
  d.setTextColor(0x07FF, 0x0000);
  d.setTextSize(2); d.setCursor(6, 5); d.print("HOTSPOT ADV");
  d.setTextSize(1); d.setTextColor(0xFFFF, 0x0000);
  d.setCursor(6, 28); d.printf("Wi-Fi: %s", apRunning ? "ATIVO" : "DESLIGADO");
  d.setCursor(6, 40); d.printf("SD: %s", sdOK ? "OK" : "NAO DETECTADO");
  d.setCursor(6, 52); d.printf("Site: %.25s", selectedSite.c_str());
  for (int i=0;i<MENU_COUNT;i++) {
    int y=72+i*25;
    if (i==menuIndex) { d.fillRoundRect(3,y-2,234,22,3,0x03EF); d.setTextColor(0x0000,0x03EF); }
    else d.setTextColor(0xFFFF,0x0000);
    d.setCursor(8,y+3); d.printf("%c %s", i==menuIndex?'>':' ', menuItems[i]);
  }
  d.setTextColor(0xFFE0,0x0000); d.setCursor(6,226); d.print("Setas: mover | Enter: selecionar");
  d.setCursor(6,238); d.print("No celular: http://192.168.4.1");
}
void saveConfig() {
  if (!sdOK) return;
  SD.remove("/hotspot-adv.cfg");
  File f=SD.open("/hotspot-adv.cfg", FILE_WRITE);
  if (!f) return;
  f.printf("ssid=%s\npassword=%s\nsite=%s\n",ssid,password,selectedSite.c_str()); f.close();
}
void loadConfig() {
  if (!sdOK || !SD.exists("/hotspot-adv.cfg")) return;
  File f=SD.open("/hotspot-adv.cfg", FILE_READ); if(!f) return;
  while(f.available()) {
    String line=f.readStringUntil('\n'); line.trim(); int p=line.indexOf('='); if(p<1) continue;
    String k=line.substring(0,p), v=line.substring(p+1);
    if(k=="ssid" && v.length()>0 && v.length()<=32) strlcpy(ssid,v.c_str(),sizeof(ssid));
    if(k=="password" && v.length()>=8 && v.length()<sizeof(password)) strlcpy(password,v.c_str(),sizeof(password));
    if(k=="site") selectedSite=trimPath(v);
  } f.close();
}
String htmlEscape(String s) { s.replace("&","&amp;"); s.replace("<","&lt;"); s.replace(">","&gt;"); s.replace("\"","&quot;"); return s; }
void sendPortal() {
  String p=selectedSite + "/index.html";
  if (sdOK && SD.exists(p)) { File f=SD.open(p,FILE_READ); server.streamFile(f,"text/html; charset=utf-8"); f.close(); return; }
  server.send(200,"text/html; charset=utf-8","<!doctype html><html><meta name='viewport' content='width=device-width'><title>Hotspot ADV</title><body style='font:18px sans-serif;max-width:700px;margin:3em auto;padding:1em'><h1>HOTSPOT ADV UNIVERSAL</h1><p>Rede local ativa. Nenhum index.html foi encontrado na pasta escolhida.</p><p>Coloque seu site em <code>"+htmlEscape(selectedSite)+"/index.html</code> no microSD.</p><p>Endereço local: <a href='http://192.168.4.1/'>192.168.4.1</a></p></body></html>");
}
void handleFile() {
  String uri=server.uri(); if(uri=="/") { sendPortal(); return; }
  uri=trimPath(uri);
  // Serve files only from selected site directory; traversal is rejected by trimPath.
  String full=selectedSite+uri;
  if (full.indexOf("..")>=0 || !full.startsWith(selectedSite+"/")) { server.send(400,"text/plain","Bad path"); return; }
  if(!sdOK || !SD.exists(full)) { server.send(404,"text/plain; charset=utf-8","Arquivo nao encontrado no site selecionado."); return; }
  File f=SD.open(full,FILE_READ); if(!f || f.isDirectory()) { if(f) f.close(); server.send(404,"text/plain","Not found"); return; }
  server.streamFile(f,mimeType(full)); f.close();
}
void startPortal() {
  if(apRunning) { dns.stop(); server.stop(); WiFi.softAPdisconnect(true); WiFi.mode(WIFI_OFF); apRunning=false; statusLine="Hotspot desligado"; return; }
  if(strlen(password)<8) strlcpy(password,"12345678",sizeof(password));
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP,apIP,IPAddress(255,255,255,0));
  bool ok=WiFi.softAP(ssid,password,1,false,10);
  if(!ok) { statusLine="Falha ao iniciar Wi-Fi"; WiFi.mode(WIFI_OFF); return; }
  dns.start(53,"*",apIP);
  server.on("/generate_204",HTTP_GET,sendPortal); server.on("/hotspot-detect.html",HTTP_GET,sendPortal);
  server.on("/connecttest.txt",HTTP_GET,sendPortal); server.on("/ncsi.txt",HTTP_GET,sendPortal);
  server.onNotFound(handleFile); server.begin(); apRunning=true; statusLine="Hotspot ativo";
}
void listSites() {
  auto &d=M5Cardputer.Display; d.fillScreen(TFT_BLACK); d.setTextColor(TFT_CYAN); d.setTextSize(2); d.setCursor(5,5); d.println("ESCOLHER SITE");
  d.setTextSize(1); d.setTextColor(TFT_WHITE); d.setCursor(5,30); d.println("Pastas em /sites; digite o nome:");
  d.setCursor(5,48); d.println("Exemplo: exemplo"); d.setCursor(5,65); d.println("Digite nome + Enter; Esc cancela");
  String input=""; bool done=false;
  while(!done) {
    M5Cardputer.update();
    if(M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
      auto s=M5Cardputer.Keyboard.keysState();
      if(s.enter) { done=true; break; }
      if(s.del && input.length()) input.remove(input.length()-1);
      for(auto c:s.word) if(isalnum(c)||c=='_'||c=='-') input+=(char)c;
      for(auto c:s.word) if(c=='q' || c=='Q') { input=""; done=true; break; }
      d.fillRect(5,90,230,20,TFT_BLACK); d.setCursor(5,90); d.setTextColor(TFT_YELLOW); d.print(input);
    }
    delay(10);
  }
  if(input.length()) { String p="/sites/"+input; if(sdOK && SD.exists(p) && SD.open(p).isDirectory()) { selectedSite=p; saveConfig(); statusLine="Site selecionado"; } else statusLine="Pasta nao encontrada"; }
}
void editText(const char* title, char* target, size_t maxLen, bool isPassword) {
  auto &d=M5Cardputer.Display; String input=target; d.fillScreen(TFT_BLACK); d.setTextColor(TFT_CYAN); d.setTextSize(2); d.setCursor(5,5); d.println(title); d.setTextSize(1); d.setTextColor(TFT_WHITE); d.setCursor(5,32); d.println("Digite e pressione Enter"); d.setCursor(5,46); d.println("Esc cancela; Backspace apaga");
  bool done=false, cancel=false;
  while(!done) { M5Cardputer.update(); if(M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) { auto s=M5Cardputer.Keyboard.keysState(); if(s.enter) done=true; for(auto c:s.word) if(c=='q' || c=='Q') {cancel=true;done=true;} if(s.del && input.length()) input.remove(input.length()-1); for(auto c:s.word) if(c>=32 && c<127 && input.length()<maxLen) input+=(char)c; d.fillRect(5,75,230,25,TFT_BLACK); d.setCursor(5,78); d.setTextColor(TFT_YELLOW); d.print(isPassword?String('*',input.length()):input); } delay(10); }
  if(!cancel && input.length() && (!isPassword || input.length()>=8)) { strlcpy(target,input.c_str(),maxLen+1); saveConfig(); }
}
void showConnections() {
  auto &d=M5Cardputer.Display; d.fillScreen(TFT_BLACK); d.setTextColor(TFT_CYAN); d.setTextSize(2); d.setCursor(5,5); d.println("DISPOSITIVOS"); d.setTextSize(1); d.setTextColor(TFT_WHITE); d.setCursor(5,35); d.printf("Associados ao Wi-Fi: %d",WiFi.softAPgetStationNum()); d.setCursor(5,50); d.printf("Rede: %s",ssid); d.setCursor(5,65); d.println("IP: 192.168.4.1"); d.setCursor(5,90); d.println("A lista de IPs individuais nao e" ); d.setCursor(5,103); d.println("exposta pela API padrao do AP." ); d.setCursor(5,220); d.println("Pressione Esc para voltar");
  while(true) { M5Cardputer.update(); if(M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) { auto s=M5Cardputer.Keyboard.keysState(); if(s.enter) break; for(auto c:s.word) if(c=='q'||c=='Q') return; } delay(10); }
}
void setup() {
  auto cfg=M5.config(); M5Cardputer.begin(cfg,true); M5Cardputer.Display.setRotation(1);
  M5Cardputer.Display.fillScreen(TFT_BLACK); M5Cardputer.Display.setTextColor(TFT_WHITE); M5Cardputer.Display.setTextSize(2); M5Cardputer.Display.setCursor(5,5); M5Cardputer.Display.println("HOTSPOT ADV"); M5Cardputer.Display.setTextSize(1); M5Cardputer.Display.println("Inicializando cartao SD...");
  sdSPI.begin(40,39,14,12); sdOK=SD.begin(12,sdSPI,20000000); if(sdOK) { if(!SD.exists("/sites")) SD.mkdir("/sites"); loadConfig(); }
  statusLine=sdOK?"Cartao SD pronto":"Cartao SD nao detectado"; drawScreen();
}
void loop() {
  M5Cardputer.update();
  if(apRunning) { dns.processNextRequest(); server.handleClient(); }
  if(M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
    auto s=M5Cardputer.Keyboard.keysState();
    for(auto c:s.word) { if(c=='s' || c=='S' || c=='j' || c=='J') { menuIndex=(menuIndex+1)%MENU_COUNT; drawScreen(); } if(c=='w' || c=='W' || c=='k' || c=='K') { menuIndex=(menuIndex+MENU_COUNT-1)%MENU_COUNT; drawScreen(); } }
    if(s.enter) {
      switch(menuIndex) {
        case 0: startPortal(); break;
        case 1: listSites(); break;
        case 2: editText("NOME DA REDE",ssid,32,false); break;
        case 3: editText("SENHA (8+)",password,63,true); break;
        case 4: showConnections(); break;
        case 5: statusLine=sdOK?"Cartao SD OK":"Cartao SD nao detectado"; break;
      }
      drawScreen();
    }
    for(auto c:s.word) if((c=='q'||c=='Q') && apRunning) { startPortal(); drawScreen(); }
  }
  delay(2);
}
