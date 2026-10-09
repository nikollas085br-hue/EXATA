#include <M5Cardputer.h>
#include <cmath>
#include <vector>
#include <algorithm>

// EXATA — calculadora científica + gráfico. Parser limitado à RAM do ESP32-S3.
// Equações em uma variável são resolvidas numericamente; não há garantia para toda equação.

static constexpr int SW=240, SH=135;
static constexpr uint16_t BG=0x0841, PANEL=0x18E3, ACCENT=0x07FF, TXT=0xFFFF, MUTED=0xBDF7, ROOT=0xF800;
enum ScreenMode { HOME, CALC, GRAPH, SHORTCUTS, HELP };
ScreenMode screenMode=HOME;
double xMin=-10.0,xMax=10.0,yMin=-10.0,yMax=20.0;
String expr="";
String result="Digite uma expressao";
String lastShortcut="Nenhum";
int cursorPos=0;
bool fractionDenominator=false;
bool angleDegrees=true;

void header(const char* t){
 M5.Display.fillRect(0,0,SW,18,PANEL); M5.Display.setTextSize(1);
 M5.Display.setTextColor(ACCENT,PANEL); M5.Display.setCursor(5,5); M5.Display.print("EXATA");
 M5.Display.setTextColor(TXT,PANEL); M5.Display.setCursor(58,5); M5.Display.print(t);
}

// Recursive-descent expression evaluator: + - * / ^, parentheses, x, pi, e,
// sin/cos/tan, asin/acos/atan, sqrt, log/log10, ln, abs, exp.
struct Parser {
 String s; int p=0; double x=0; bool ok=true;
 Parser(const String& text,double xv=0):s(text),x(xv){}
 void ws(){while(p<(int)s.length() && s[p]==' ')p++;}
 bool eat(char c){ws();if(p<(int)s.length()&&s[p]==c){p++;return true;}return false;}
 String ident(){ws();int st=p;while(p<(int)s.length()&&((s[p]>='a'&&s[p]<='z')||(s[p]>='A'&&s[p]<='Z')))p++;String q=s.substring(st,p);q.toLowerCase();return q;}
 double expression(){double v=term();while(ok){if(eat('+'))v+=term();else if(eat('-'))v-=term();else break;}return v;}
 double term(){
  double v=power();
  while(ok){
   if(eat('*'))v*=power();
   else if(eat('/')){double d=power();if(fabs(d)<1e-15){ok=false;return NAN;}v/=d;}
   else {
    ws(); char c=(p<(int)s.length())?s[p]:'\0';
    // Multiplicacao implicita: 2x, 3(x+1), 2pi, 2sin(x).
    if((c=='x'||c=='X'||c=='(')||((c>='a'&&c<='z')||(c>='A'&&c<='Z'))) v*=power();
    else break;
   }
  }
  return v;
 }
 double power(){double v=unary();if(eat('^'))v=pow(v,power());return v;}
 double unary(){if(eat('+'))return unary();if(eat('-'))return -unary();return atom();}
 double atom(){
  ws();if(eat('(')){double v=expression();if(!eat(')'))ok=false;return v;}
  if(p<(int)s.length()&&((s[p]>='0'&&s[p]<='9')||s[p]=='.')){
   const char* start=s.c_str()+p;char* end=nullptr;double v=strtod(start,&end);if(end==start){ok=false;return NAN;}p+=(end-start);return v;
  }
  String id=ident();
  if(id.length()){
   if(id=="x")return x;if(id=="pi")return 3.14159265358979323846;if(id=="e")return 2.71828182845904523536;
   if(!eat('(')){ok=false;return NAN;}double a=expression();if(!eat(')'))ok=false;
   if(id=="sin")return sin(angleDegrees?a*0.017453292519943295:a);
   if(id=="cos")return cos(angleDegrees?a*0.017453292519943295:a);
   if(id=="tan")return tan(angleDegrees?a*0.017453292519943295:a);
   if(id=="asin"){double v=asin(a);return angleDegrees?v*57.29577951308232:v;}
   if(id=="acos"){double v=acos(a);return angleDegrees?v*57.29577951308232:v;}
   if(id=="atan"){double v=atan(a);return angleDegrees?v*57.29577951308232:v;}
   if(id=="sqrt"){if(a<0){ok=false;return NAN;}return sqrt(a);}
   if(id=="ln"){if(a<=0){ok=false;return NAN;}return log(a);}
   if(id=="log"||id=="log10"){if(a<=0){ok=false;return NAN;}return log10(a);}
   if(id=="abs")return fabs(a);if(id=="exp")return exp(a);
   ok=false;return NAN;
  }
  ok=false;return NAN;
 }
 double run(){double v=expression();ws();if(p!=(int)s.length())ok=false;if(!ok||!isfinite(v))return NAN;return v;}
};
bool splitEquation(String in,String &left,String &right){
 int at=in.indexOf('=');if(at<0)return false;left=in.substring(0,at);right=in.substring(at+1);return left.length()&&right.length();
}
double equationValue(const String& l,const String& r,double x,bool &ok){
 Parser a(l,x),b(r,x);double av=a.run(),bv=b.run();ok=isfinite(av)&&isfinite(bv);return av-bv;
}
String calculate(String in){
 in.trim();if(!in.length())return "Digite uma expressao";
 String l,r;
 if(splitEquation(in,l,r)){
  const double lo=-100.0, hi=100.0, step=0.25;
  std::vector<double> roots;
  bool prevOk=false;double prevX=lo;bool ok=false;double prev=equationValue(l,r,prevX,ok);prevOk=ok;
  for(double xx=lo+step;xx<=hi+1e-9;xx+=step){
   bool curOk=false;double cur=equationValue(l,r,xx,curOk);
   if(curOk&&prevOk&&prev*cur<=0){
    double a=prevX,b=xx,fa=prev;bool valid=true;
    for(int k=0;k<55;k++){double m=(a+b)/2;bool om=false;double fm=equationValue(l,r,m,om);if(!om){valid=false;break;}if(fabs(fm)<1e-10){a=b=m;break;}if(fa*fm<=0)b=m;else{a=m;fa=fm;}}
    if(valid){double root=(a+b)/2;bool fresh=true;for(double old:roots)if(fabs(old-root)<1e-3){fresh=false;break;}if(fresh&&roots.size()<8)roots.push_back(root);}
   }
   prevX=xx;prev=cur;prevOk=curOk;
  }
  if(roots.empty())return "Sem raiz detectada (-100 a 100)";
  String out="";for(size_t i=0;i<roots.size();i++){if(i)out+=" ";out+="x"+String((int)i+1)+"="+String(roots[i],3);}return out;
 }
 Parser p(in);double v=p.run();if(!isfinite(v))return "Erro: sintaxe/dominio";return String(v,8);
}
int mapX(double x){return int((x-xMin)/(xMax-xMin)*(SW-1));}
int mapY(double y){return int((SH-1)-(y-yMin)/(yMax-yMin)*(SH-1));}
double graphF(double x){
 String left,right;
 if(splitEquation(expr,left,right)){bool ok=false;return equationValue(left,right,x,ok);}
 Parser p(expr,x);return p.run();
}

void drawHome(){
 M5.Display.fillScreen(BG);header("CALCULADORA UNIVERSAL");
 M5.Display.setTextColor(ACCENT,BG);M5.Display.setTextSize(2);M5.Display.setCursor(12,27);M5.Display.print("EXATA");
 M5.Display.setTextSize(1);M5.Display.setTextColor(TXT,BG);
 M5.Display.setCursor(12,51);M5.Display.print("Matematica cientifica");
 M5.Display.drawRoundRect(7,65,109,25,4,ACCENT);M5.Display.setCursor(16,74);M5.Display.print("ENTER Calculadora");
 M5.Display.drawRoundRect(124,65,108,25,4,ACCENT);M5.Display.setCursor(133,74);M5.Display.print("G Grafico");
 M5.Display.setCursor(8,101);M5.Display.print("M atalhos  H ajuda  F fracao");
 M5.Display.setTextColor(MUTED,BG);M5.Display.setCursor(8,119);M5.Display.print("Digite expressoes e pressione ENTER");
}
void drawCalc(){
 M5.Display.fillScreen(BG);header("CALCULADORA");
 M5.Display.setTextColor(MUTED,BG);M5.Display.setCursor(5,23);M5.Display.print("EXPRESSAO (x para variavel)");
 M5.Display.setTextColor(ACCENT,BG);M5.Display.setCursor(5,38);
 // Janela horizontal acompanha o cursor para permitir editar expressoes longas.
 int start=cursorPos-28;if(start<0)start=0;
 String shown=expr.substring(start,start+34);
 M5.Display.print(shown);
 int cx=5+(cursorPos-start)*6;
 if(cx>5+34*6)cx=5+34*6;
 M5.Display.drawFastVLine(cx,36,12,YELLOW);
 M5.Display.setTextColor(TXT,BG);
 M5.Display.setCursor(5,56);M5.Display.print("Resultado:");
 M5.Display.setTextColor(ACCENT,BG);M5.Display.setCursor(5,69);M5.Display.print(result);
 M5.Display.setTextColor(MUTED,BG);M5.Display.setCursor(5,91);M5.Display.print("F fracao  > denominador");
 M5.Display.setCursor(5,105);M5.Display.print("S sin C cos T tan R sqrt P pi");
 M5.Display.setCursor(5,119);M5.Display.print("ENTER resolver | ESC volta | DEL apaga");
}
void drawGraph(){
 M5.Display.fillScreen(BG);
 int zy=mapY(0),zx=mapX(0);if(zy>=18&&zy<SH)M5.Display.drawFastHLine(0,zy,SW,0x4208);if(zx>=0&&zx<SW)M5.Display.drawFastVLine(zx,18,SH-18,0x4208);
 int px0=-1,py0=-1;for(int px=0;px<SW;px++){double x=xMin+double(px)/(SW-1)*(xMax-xMin),y=graphF(x);if(!isfinite(y)){px0=-1;continue;}int py=mapY(y);if(py>=18&&py<SH){if(px0>=0&&abs(py-py0)<SH/2)M5.Display.drawLine(px0,py0,px,py,ACCENT);else M5.Display.drawPixel(px,py,ACCENT);px0=px;py0=py;}else px0=-1;}
 header("GRAFICO DA EXPRESSAO");M5.Display.setTextColor(MUTED,BG);M5.Display.setCursor(4,120);M5.Display.print("WASD mover +/- zoom ESC volta");
}
void drawShortcuts(){
 M5.Display.fillScreen(BG);header("ATALHOS");
 M5.Display.setTextColor(TXT,BG);
 M5.Display.setCursor(7,25);M5.Display.print("S sin(x)   C cos(x)   T tan(x)");
 M5.Display.setCursor(7,40);M5.Display.print("R sqrt(x)  L log(x)   N ln(x)");
 M5.Display.setCursor(7,55);M5.Display.print("P pi        E constante e");
 M5.Display.setCursor(7,70);M5.Display.print("F inserir modelo de fracao");
 M5.Display.setCursor(7,88);M5.Display.setTextColor(ACCENT,BG);M5.Display.print(lastShortcut);
 M5.Display.setTextColor(MUTED,BG);M5.Display.setCursor(7,119);M5.Display.print("M volta | ENTER abre calculadora");
}
void drawHelp(){M5.Display.fillScreen(BG);header("AJUDA");M5.Display.setTextColor(TXT,BG);
 M5.Display.setCursor(5,27);M5.Display.print("+ - * / ^ e parenteses");
 M5.Display.setCursor(5,42);M5.Display.print("Funcoes: sin cos tan sqrt log ln");
 M5.Display.setCursor(5,57);M5.Display.print("Equacao: 2*x+3=9 (resolve x numerico)");
 M5.Display.setCursor(5,72);M5.Display.print("F fracao; > vai denominador; Q =");
 M5.Display.setCursor(5,87);M5.Display.print("Trigonometria em graus por padrao");
 M5.Display.setTextColor(MUTED,BG);M5.Display.setCursor(5,119);M5.Display.print("ESC/M volta | ENTER calcula");}
void redraw(){if(screenMode==HOME)drawHome();else if(screenMode==CALC)drawCalc();else if(screenMode==GRAPH)drawGraph();else if(screenMode==SHORTCUTS)drawShortcuts();else drawHelp();}
void insertText(String t){expr=expr.substring(0,cursorPos)+t+expr.substring(cursorPos);cursorPos+=t.length();}
void handleKey(char k){
 // Controles de cursor: setas ASCII/teclas de navegacao quando reportadas pelo firmware.
 if(screenMode==CALC && (k=='<' || k=='[' || (unsigned char)k==0x1C)) {if(cursorPos>0)cursorPos--;redraw();return;}
 if(screenMode==CALC && (k==']' || (unsigned char)k==0x1D)) {if(cursorPos<(int)expr.length())cursorPos++;redraw();return;}
 if(k>='a'&&k<='z')k-=32;
 if(k==27){screenMode=(screenMode==GRAPH||screenMode==HELP||screenMode==SHORTCUTS)?CALC:HOME;redraw();return;}
 if(k=='M'){screenMode=(screenMode==HOME)?SHORTCUTS:(screenMode==CALC?HOME:CALC);redraw();return;}
 if(k=='H'){screenMode=HELP;redraw();return;}
 if(k=='G'){screenMode=GRAPH;redraw();return;}
 if(k=='F'&&screenMode==CALC){insertText("( )/( )");cursorPos-=5;fractionDenominator=false;redraw();return;}
 if(screenMode==HOME&&k=='\n'){screenMode=CALC;redraw();return;}
 if(screenMode==CALC){
  if(k=='\n'||k=='='){result=calculate(expr);redraw();return;}
  if(k=='Q'){insertText("=");redraw();return;}
  if(k==8||k==127){if(cursorPos>0){expr.remove(cursorPos-1,1);cursorPos--;}redraw();return;}
  if(k=='>'){int d=expr.indexOf(")/(");if(d>=0){cursorPos=d+2;fractionDenominator=true;}else if(cursorPos<(int)expr.length())cursorPos++;redraw();return;}
  if(k=='S'){insertText("sin()");cursorPos--;lastShortcut="sin(x)";}
  else if(k=='C'){insertText("cos()");cursorPos--;}
  else if(k=='T'){insertText("tan()");cursorPos--;}
  else if(k=='R'){insertText("sqrt()");cursorPos--;}
  else if(k=='L'){insertText("log()");cursorPos--;}
  else if(k=='N'){insertText("ln()");cursorPos--;}
  else if(k=='P'){insertText("pi");}
  else if(k=='E'){insertText("e");}
  else if(k==' '){insertText(" ");}
  else if((k>='0'&&k<='9')||k=='.'||k=='+'||k=='-'||k=='*'||k=='/'||k=='^'||k=='('||k==')'||k=='='||k=='x'||k=='X'){insertText(String(k));}
  redraw();return;
 }
 if(screenMode==GRAPH){
  if(k==8||k==127){screenMode=CALC;redraw();return;}
  if(k=='W'||k=='S'){double d=(yMax-yMin)*.1;if(k=='W'){yMin+=d;yMax+=d;}else{yMin-=d;yMax-=d;}}
  if(k=='A'||k=='D'){double d=(xMax-xMin)*.1;if(k=='A'){xMin-=d;xMax-=d;}else{xMin+=d;xMax+=d;}}
  if(k=='+'||k=='-'){double d=(xMax-xMin)*.2;if(k=='+'){xMin+=d;xMax-=d;}else{xMin-=d;xMax+=d;}}
 }
 if(screenMode==SHORTCUTS){if(k=='C')lastShortcut="cos(x)";else if(k=='T')lastShortcut="tan(x)";else if(k=='S')lastShortcut="sin(x)";else if(k=='R')lastShortcut="sqrt(x)";else if(k=='P')lastShortcut="pi";else if(k=='F')lastShortcut="fracao";}
 redraw();
}
void setup(){auto cfg=M5.config();M5Cardputer.begin(cfg,true);M5.Display.setRotation(1);M5.Display.setTextSize(1);redraw();}
void loop(){
 M5Cardputer.update();
 if(M5Cardputer.Keyboard.isChange()&&M5Cardputer.Keyboard.isPressed()){
  auto keys=M5Cardputer.Keyboard.keysState();
  for(auto k:keys.word)handleKey(k);
  if(keys.enter)handleKey('\n');
  if(keys.del){handleKey(8);}
 }
 delay(20);
}
