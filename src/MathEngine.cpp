#include "MathEngine.h"
#include <math.h>

namespace MathEngine {
class Parser {
    String s; size_t p=0; double xv=0;
    void skip(){while(p<s.length() && (s[p]==' '||s[p]=='\t'))p++;}
    bool eat(char c){skip(); if(p<s.length()&&s[p]==c){p++;return true;} return false;}
    double number(){skip(); size_t st=p; bool dot=false; while(p<s.length()) {char c=s[p]; if(isDigit(c)) p++; else if(c=='.'&&!dot){dot=true;p++;} else break;} if(st==p)return NAN; return s.substring(st,p).toDouble();}
    String ident(){skip();size_t st=p;while(p<s.length()&&(isAlphaNumeric(s[p])||s[p]=='_'))p++;return s.substring(st,p);}
    double primary(){
        skip();
        if(eat('(')){double v=expr();if(!eat(')'))return NAN;return v;}
        if(p<s.length()&&(s[p]=='+'||s[p]=='-')){char c=s[p++];double v=primary();return c=='-'?-v:v;}
        if(p<s.length()&&isDigit(s[p]))return number();
        if(p<s.length()&&(isAlpha(s[p])||s[p]=='_')){
            String id=ident();id.toLowerCase();
            if(id=="x")return xv;
            if(id=="pi"||id=="p")return PI;
            if(id=="e")return M_E;
            if(!eat('('))return NAN;
            double a=expr();if(!eat(')'))return NAN;
            if(id=="sin"||id=="sen")return sin(a);
            if(id=="cos")return cos(a);
            if(id=="tan"||id=="tg")return tan(a);
            if(id=="asin"||id=="arcsin")return asin(a);
            if(id=="acos"||id=="arccos")return acos(a);
            if(id=="atan"||id=="arctan")return atan(a);
            if(id=="sqrt"||id=="raiz")return sqrt(a);
            if(id=="abs"||id=="mod")return fabs(a);
            if(id=="ln")return log(a);
            if(id=="log"||id=="log10")return log10(a);
            if(id=="exp")return exp(a);
            if(id=="sinh")return sinh(a);
            if(id=="cosh")return cosh(a);
            if(id=="tanh")return tanh(a);
            if(id=="floor")return floor(a);
            if(id=="ceil")return ceil(a);
            return NAN;
        }
        return NAN;
    }
    double power(){double v=primary();skip();if(eat('^')){double r=power();return pow(v,r);}return v;}
    bool startsPrimary(){skip();if(p>=s.length())return false;char c=s[p];return isDigit(c)||isAlpha(c)||c=='_'||c=='(';}
    double term(){double v=power();while(true){skip();if(eat('*'))v*=power();else if(eat('/')){double d=power();if(fabs(d)<1e-15)return NAN;v/=d;}else if(startsPrimary())v*=power();else break;}return v;}
    double expr(){double v=term();while(true){skip();if(eat('+'))v+=term();else if(eat('-'))v-=term();else break;}return v;}
public: Parser(const String&a,double x):s(a),xv(x){} double run(){double v=expr();skip();return p==s.length()?v:NAN;}
};
static String fmt(double v){if(!isfinite(v))return "indefinido";if(fabs(v)<1e-10)v=0;if(fabs(v-round(v))<1e-9)return String((long long)round(v));return String(v,7);}
Result evaluate(const String&e,double x){String s=e;s.trim();if(!s.length())return {false,NAN,"Digite uma expressao."};Parser p(s,x);double v=p.run();if(!isfinite(v))return {false,v,"Expressao invalida."};return {true,v,fmt(v)};}
static bool splitEq(const String&eq,String&L,String&R){int p=eq.indexOf('=');if(p<0)return false;if(eq.indexOf('=',p+1)>=0)return false;L=eq.substring(0,p);R=eq.substring(p+1);L.trim();R.trim();return L.length()&&R.length();}
static double fEq(const String&l,const String&r,double x,bool&ok){auto a=evaluate(l,x),b=evaluate(r,x);ok=a.ok&&b.ok;return ok?a.value-b.value:NAN;}
Result solveLinear(double a,double b){if(fabs(a)<1e-12)return {false,NAN,fabs(b)<1e-12?"Infinitas solucoes.":"Sem solucao."};double x=-b/a;return {true,x,"Equacao de 1o grau\n"+fmt(a)+"x + "+fmt(b)+" = 0\nx = "+fmt(x)};}
Result solveQuadratic(double a,double b,double c){if(fabs(a)<1e-12)return solveLinear(b,c);double d=b*b-4*a*c;if(d<0)return {false,NAN,"2o grau\nDelta = "+fmt(d)+"\nSem raizes reais."};double r=sqrt(d),x1=(-b+r)/(2*a),x2=(-b-r)/(2*a);if(fabs(x1-x2)<1e-9)return {true,x1,"2o grau\nDelta = "+fmt(d)+"\nx = "+fmt(x1)};return {true,x1,"2o grau\nDelta = "+fmt(d)+"\nx1 = "+fmt(x1)+"\nx2 = "+fmt(x2)};}
Result solveEquation(const String&equation){
 String l,r;if(!splitEq(equation,l,r))return {false,NAN,"Use uma igualdade, por exemplo: 2x+3=9"};
 bool ok=false;double c=fEq(l,r,0,ok);if(!ok)return {false,NAN,"Nao consegui interpretar a equacao."};
 double f1=fEq(l,r,1,ok);if(!ok)return {false,NAN,"Nao consegui interpretar a equacao."};
 double f2=fEq(l,r,2,ok);if(!ok)return {false,NAN,"Nao consegui interpretar a equacao."};
 double f3=fEq(l,r,3,ok);if(!ok)return {false,NAN,"Nao consegui interpretar a equacao."};
 double a=(f2-2*f1+c)/2.0,b=f1-c-a;
 if(fabs(f3-(9*a+3*b+c))<1e-5 && fabs(a)<1e-9)return solveLinear(b,c);
 if(fabs(f3-(9*a+3*b+c))<1e-5)return solveQuadratic(a,b,c);
 // numerical search for any real root
 double lastX=-50,last=fEq(l,r,lastX,ok); if(!ok)return {false,NAN,"Equacao invalida."};
 double best=999999;double bestX=NAN;
 for(int i=1;i<=800;i++){double x=-50+i*0.125;double y=fEq(l,r,x,ok);if(ok&&isfinite(y)){if(fabs(y)<best){best=fabs(y);bestX=x;}if(isfinite(last)&&((last<0&&y>0)||(last>0&&y<0))){double lo=lastX,hi=x,fl=last;for(int j=0;j<55;j++){double mid=(lo+hi)/2,fm=fEq(l,r,mid,ok);if(!ok)break;if(fabs(fm)<1e-10){lo=hi=mid;break;}if((fl<0&&fm>0)||(fl>0&&fm<0)){hi=mid;}else{lo=mid;fl=fm;}}double root=(lo+hi)/2;return {true,root,"Equacao resolvida numericamente\nx = "+fmt(root)};}last=y;lastX=x;}}
 if(isfinite(bestX)&&best<1e-5)return {true,bestX,"Raiz aproximada\nx = "+fmt(bestX)};
 return {false,NAN,"Nao encontrei uma raiz real no intervalo testado."};
}
Result solveExponential(const String&equation){Result r=solveEquation(equation);if(r.ok)r.text="EXPONENCIAL\n"+r.text;return r;}
String describeExpression(const String&e){return "f(x) = "+e;}
}
