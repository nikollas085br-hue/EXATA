#include <M5Cardputer.h>
#include <cmath>
#include <complex>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>

using C = std::complex<double>;
static const double PI = 3.14159265358979323846;

String expr;
String answer;
String detail;
bool showHelp = false;
bool showSteps = false;
bool radians = true;
int scroll = 0;

struct Parser {
  String s;
  int p = 0;
  double xValue = 0.0;
  bool hasX = false;
  String err = "";
  String steps = "";

  Parser(const String& input, double xv=0.0, bool hx=false) : s(input), xValue(xv), hasX(hx) {}

  void ws() { while (p < s.length() && s[p] == ' ') p++; }
  bool eat(char c) { ws(); if (p < s.length() && s[p] == c) { p++; return true; } return false; }
  bool starts(const char* t) { ws(); String z=t; if (s.substring(p, p+z.length()).equalsIgnoreCase(z)) { p += z.length(); return true; } return false; }
  C parse() { C v = add(); ws(); if (p < s.length() && err == "") err = String("Símbolo inesperado: ") + s[p]; return v; }

  C add() {
    C a = mul();
    while (err == "") {
      if (eat('+')) { C b=mul(); C old=a; a+=b; steps += String(old.real(),6)+" + "+String(b.real(),6)+" = "+String(a.real(),6)+"\n"; }
      else if (eat('-')) { C b=mul(); C old=a; a-=b; steps += String(old.real(),6)+" - "+String(b.real(),6)+" = "+String(a.real(),6)+"\n"; }
      else break;
    }
    return a;
  }
  C mul() {
    C a = power();
    while (err == "") {
      if (eat('*')) { C b=power(); C old=a; a*=b; steps += "Multiplicação: "+String(old.real(),6)+" × "+String(b.real(),6)+" = "+String(a.real(),6)+"\n"; }
      else if (eat('/')) { C b=power(); if (std::abs(b)<1e-15) {err="Divisão por zero"; return 0.;} C old=a; a/=b; steps += "Divisão: "+String(old.real(),6)+" ÷ "+String(b.real(),6)+" = "+String(a.real(),6)+"\n"; }
      else if (implicitMul()) { C b=power(); a*=b; }
      else break;
    }
    return a;
  }
  bool implicitMul() {
    ws();
    if (p >= s.length()) return false;
    char c=s[p];
    return c=='(' || isAlpha(c);
  }
  C power() {
    C a = unary();
    if (eat('^')) {
      C b = power();
      C old=a;
      a=std::pow(a,b);
      steps += "Potência: ("+String(old.real(),6)+") ^ ("+String(b.real(),6)+") = "+String(a.real(),6)+"\n";
    }
    return a;
  }
  C unary() {
    if (eat('+')) return unary();
    if (eat('-')) return -unary();
    return atom();
  }
  bool isAlpha(char c) { return (c>='a'&&c<='z') || (c>='A'&&c<='Z'); }
  String identifier() {
    ws(); int st=p;
    while(p<s.length() && isAlpha(s[p])) p++;
    return s.substring(st,p);
  }
  C atom() {
    ws();
    if (eat('(')) { C v=add(); if(!eat(')') && err=="") err="Falta )"; return v; }
    if (p >= s.length()) {err="Expressão incompleta"; return 0.;}
    if ((s[p]>='0'&&s[p]<='9') || s[p]=='.') {
      int st=p;
      while(p<s.length() && ((s[p]>='0'&&s[p]<='9') || s[p]=='.')) p++;
      if(p<s.length() && (s[p]=='e'||s[p]=='E')) {
        int q=p++; if(p<s.length()&&(s[p]=='+'||s[p]=='-'))p++;
        int d=p; while(p<s.length()&&s[p]>='0'&&s[p]<='9')p++;
        if(d==p)p=q;
      }
      return C(s.substring(st,p).toDouble(),0);
    }
    if (isAlpha(s[p])) {
      String id=identifier(); id.toLowerCase();
      if (id=="pi") return C(PI,0);
      if (id=="e") return C(M_E,0);
      if (id=="i") return C(0,1);
      if (id=="x") { if(!hasX){err="Use K para resolver a equação em x"; return 0.;} return C(xValue,0); }
      if (!eat('(')) { err="Constante/função desconhecida: "+id; return 0.; }
      C a=add(); if(!eat(')')) {err="Falta ) após função"; return 0.;}
      double v=a.real(), out=0;
      if(id=="sin") out=std::sin(radians?v:v*PI/180.0);
      else if(id=="cos") out=std::cos(radians?v:v*PI/180.0);
      else if(id=="tan") out=std::tan(radians?v:v*PI/180.0);
      else if(id=="asin") out=std::asin(v);
      else if(id=="acos") out=std::acos(v);
      else if(id=="atan") out=std::atan(v);
      else if(id=="sqrt") { if(std::abs(a.imag())<1e-12 && v<0) return std::sqrt(a); return std::sqrt(a); }
      else if(id=="abs") return C(std::abs(a),0);
      else if(id=="ln") return std::log(a);
      else if(id=="log") return std::log10(a);
      else if(id=="exp") return std::exp(a);
      else if(id=="floor") return C(std::floor(v),0);
      else if(id=="ceil") return C(std::ceil(v),0);
      else if(id=="conj") return std::conj(a);
      else if(id=="real") return C(a.real(),0);
      else if(id=="imag") return C(a.imag(),0);
      else {err="Função não reconhecida: "+id; return 0.;}
      steps += "Aplicando "+id+"("+String(v,6)+") = "+String(out,6)+"\n";
      return C(out,0);
    }
    err=String("Caractere inválido: ")+s[p]; return 0.;
  }
};

String fmt(C z) {
  if (!std::isfinite(z.real()) || !std::isfinite(z.imag())) return "Indefinido/fora do limite";
  auto clean=[](double v){return std::abs(v)<1e-10?0.0:v;};
  double re=clean(z.real()), im=clean(z.imag());
  String r=String(re,10); r.trim();
  if(std::abs(im)<1e-10) return r;
  String ii=String(std::abs(im),10);
  if(std::abs(re)<1e-10) return (im<0?"-":"")+ii+"i";
  return r+(im<0?" - ":" + ")+ii+"i";
}

bool equationParts(String in, String &left, String &right) {
  int k=in.indexOf('=');
  if(k<0) return false;
  if(in.indexOf('=',k+1)>=0) return false;
  left=in.substring(0,k); right=in.substring(k+1); left.trim(); right.trim();
  return left.length() && right.length();
}
String solveEquation(String input, String &stepsOut) {
  String l,r;
  if(!equationParts(input,l,r)) return "Digite uma equação com =";
  // Numeric scan for a sign change, then bisection. This is a numerical solver, not a proof of all roots.
  auto f=[&](double x, bool &ok)->double {
    Parser a(l,x,true), b(r,x,true); C av=a.parse(), bv=b.parse();
    ok=(a.err==""&&b.err==""&&std::abs(av.imag())<1e-8&&std::abs(bv.imag())<1e-8);
    return av.real()-bv.real();
  };
  const double bound=1e6;
  double found[12]; int count=0;
  double prevX=-bound; bool ok=false; double prevY=f(prevX,ok);
  const int N=12000;
  for(int i=1;i<=N;i++){
    double x=-bound+(2*bound*i)/N; bool ok2=false; double y=f(x,ok2);
    if(ok&&ok2&&std::isfinite(prevY)&&std::isfinite(y)) {
      if(std::abs(prevY)<1e-8 && count<12) found[count++]=prevX;
      else if((prevY<0&&y>0)||(prevY>0&&y<0)||std::abs(y)<1e-8) {
        double lo=prevX,hi=x, flo=prevY;
        for(int j=0;j<70;j++){double mid=(lo+hi)/2;bool mok=false;double fm=f(mid,mok);if(!mok)break;if(std::abs(fm)<1e-12){lo=hi=mid;break;}if((flo<0&&fm>0)||(flo>0&&fm<0))hi=mid;else{lo=mid;flo=fm;}}
        double root=(lo+hi)/2; bool dupe=false; for(int k=0;k<count;k++)if(std::abs(found[k]-root)<1e-4)dupe=true;
        if(!dupe&&count<12)found[count++]=root;
      }
    }
    prevX=x;prevY=y;ok=ok2;
  }
  if(!count) return "Nenhuma raiz real encontrada no intervalo [-1000000, 1000000]. Pode haver raízes fora dele, múltiplas ou complexas.";
  stepsOut="Método numérico por varredura e bisseção.\nEquação: "+input+"\nReescrevendo: f(x) = lado esquerdo − lado direito.\n";
  String out="Raízes reais aproximadas:\n";
  for(int i=0;i<count;i++){out+="x"+String(i+1)+" ≈ "+String(found[i],10)+"\n";stepsOut+="Verificação: x ≈ "+String(found[i],10)+"\n";}
  stepsOut+="Atenção: resultado numérico aproximado; não garante encontrar todas as raízes.";
  return out;
}

void addChar(char c) { if(expr.length()<180) expr+=c; }
void addChar(const char* s) { if(expr.length()<170) expr += s; }
void backspace() { if(expr.length()) expr.remove(expr.length()-1); }
void evaluate() {
  answer=""; detail="";
  String l,r;
  if(expr.indexOf('=')>=0) { answer=solveEquation(expr,detail); return; }
  Parser p(expr); C v=p.parse();
  if(p.err!="") {answer="Erro: "+p.err; detail="Confira parênteses, operadores e nomes das funções."; return;}
  answer=fmt(v); detail="Expressão: "+expr+"\nResultado: "+answer+"\n\nPassos registrados:\n"+p.steps;
  if(p.steps=="") detail+="Sem operações intermediárias para detalhar.";
}
void drawTextLines(const String& t,int x,int y,int maxLines) {
  int start=0, line=0;
  while(start<t.length() && line<maxLines) {
    int end=t.indexOf('\n',start); if(end<0)end=t.length();
    String part=t.substring(start,end);
    while(part.length()>37) { M5Cardputer.Display.drawString(part.substring(0,37),x,y+line*14); part=part.substring(37); line++; if(line>=maxLines) return; }
    M5Cardputer.Display.drawString(part,x,y+line*14); line++; start=end+1;
  }
}
void drawUI() {
  auto &d=M5Cardputer.Display;
  d.fillScreen(0x0841);
  d.setTextColor(0x7FEF,0x0841); d.setTextSize(1);
  d.drawString("EXATA",8,5);
  d.setTextColor(0xFFFF,0x0841); d.drawString("CALCULADORA UNIVERSAL",56,5);
  d.drawFastHLine(8,20,224,0x4D9F);
  d.setTextColor(0xBDF7,0x0841); d.drawString("EXPRESSAO:",8,27);
  d.setTextColor(0xFFFF,0x0841); drawTextLines(expr,8,43,3);
  d.drawFastHLine(8,88,224,0x4D9F);
  d.setTextColor(0x7FEF,0x0841); d.drawString("RESULTADO",8,94);
  d.setTextColor(0xFFFF,0x0841); drawTextLines(answer,8,109,4);
  d.drawFastHLine(8,168,224,0x4D9F);
  d.setTextColor(0xBDF7,0x0841);
  d.drawString("ENTER calcular | DEL apagar",8,174);
  d.drawString("M atalhos | K passos/solver",8,188);
  if(showHelp) {
    d.fillRect(5,25,230,165,0x0000); d.drawRect(5,25,230,165,0x7FEF);
    d.setTextColor(0xFFFF,0x0000);
    drawTextLines("ATALHOS / FUNCOES\n+ - * / ^ ( )\nsin(x) cos(x) tan(x)\n asin(x) acos(x) atan(x)\nsqrt(x) ou tecla R + abre parenteses\nln(x) log(x) exp(x)\nabs(x) conj(x) real(x) imag(x)\npi, e, i, x; 2(3+4) = multiplicacao\nUse '=' para equacao; K tenta resolver\nAngulos: radianos por padrao.\nM fecha esta ajuda.",10,31,11);
  }
  if(showSteps) {
    d.fillRect(5,25,230,165,0x0000); d.drawRect(5,25,230,165,0x7FEF);
    d.setTextColor(0xFFFF,0x0000);
    drawTextLines(detail.length()?detail:"Calcule primeiro; depois pressione K.",10,31,11);
  }
}
void setup() {
  auto cfg=M5.config(); M5Cardputer.begin(cfg,true);
  M5Cardputer.Display.setRotation(1);
  expr=""; answer="Pronta. Digite uma expressao."; detail="";
  drawUI();
}
void loop() {
  M5Cardputer.update();
  auto ks=M5Cardputer.Keyboard.keysState();
  bool changed=false;
  if(M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
    for(char c:ks.word) {
      if(c=='m'||c=='M') {showHelp=!showHelp;showSteps=false;changed=true;}
      else if(c=='k'||c=='K') {
        if(expr.indexOf('=')>=0) {evaluate();showSteps=true;showHelp=false;}
        else {showSteps=!showSteps;showHelp=false;}
        changed=true;
      } else if(c=='r'||c=='R') {addChar("sqrt(");changed=true;}
      else if(c=='s'||c=='S') {addChar("sin(");changed=true;}
      else if(c=='t'||c=='T') {addChar("tan(");changed=true;}
      else if(c=='c'||c=='C') {addChar("cos(");changed=true;}
      else {addChar(c);changed=true;}
    }
    if(ks.enter) {evaluate();showSteps=false;showHelp=false;changed=true;}
    if(ks.del) {backspace();answer="";changed=true;}
  }
  if(changed) drawUI();
  delay(15);
}
