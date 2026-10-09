#include "Physics.h"
#include <math.h>
namespace Physics {
static bool val(const String& d,const char* key,double& out){String low=d;low.toLowerCase();String k=String(key);k.toLowerCase();int p=low.indexOf(k+"=");if(p<0)return false;p=low.indexOf('=',p)+1;while(p<low.length()&&low[p]==' ')p++;int e=p;while(e<low.length()&&low[e]!=','&&low[e]!=';'&&low[e]!=' ')e++;if(e<=p)return false;String s=low.substring(p,e);out=s.toDouble();return true;}
static String f(double x){if(!isfinite(x))return "indefinido";if(fabs(x-round(x))<1e-9)return String((long long)round(x));return String(x,5);}
const char* name(uint8_t t){switch(t){case 0:return "Forca resultante";case 1:return "Forca peso";case 2:return "Forca normal";case 3:return "Atrito";case 4:return "Forca elastica";case 5:return "Cinematica";case 6:return "Energia e trabalho";case 7:return "Densidade";default:return "Fisica";}}
String formula(uint8_t t){switch(t){case 0:return "Fr = soma das forcas";case 1:return "P = m · g";case 2:return "N = m · g · cos(theta)";case 3:return "Fat = μ · N";case 4:return "Fe = k · x";case 5:return "vf = vi + a · t";case 6:return "W = F · d · cos(theta)";case 7:return "rho = m / V";default:return "";}}
String solve(uint8_t t,const String&d){
 double m=0,a=0,F=0,F1=0,F2=0,F3=0,v=0,vi=0,vf=0,di=0,tm=0,g=0,h=0,mu=0,N=0,k=0,x=0,V=0,theta=0; bool b;
 switch(t){
  case 0:
   if(val(d,"F1",F1)||val(d,"F2",F2)||val(d,"F3",F3)){val(d,"F1",F1);val(d,"F2",F2);val(d,"F3",F3);F=F1+F2+F3;return "FORCA RESULTANTE\nFr = F1 + F2 + F3\nFr = "+f(F1)+" + "+f(F2)+" + "+f(F3)+"\nFr = "+f(F)+" N";}
   if(val(d,"m",m)&&val(d,"a",a))return "FORCA RESULTANTE\nFr = m · a\nFr = "+f(m)+" · "+f(a)+"\nFr = "+f(m*a)+" N";
   break;
  case 1:
   if(val(d,"m",m)&&val(d,"g",g))return "FORCA PESO\nP = m · g\nP = "+f(m)+" · "+f(g)+"\nP = "+f(m*g)+" N";
   break;
  case 2:
   if(val(d,"m",m)&&val(d,"g",g)){if(val(d,"theta",theta)){double rad=theta*3.14159265358979323846/180.0;N=m*g*cos(rad);return "FORCA NORMAL\nN = m · g · cos(theta)\nN = "+f(m)+" · "+f(g)+" · cos("+f(theta)+")\nN = "+f(N)+" N";}N=m*g;return "FORCA NORMAL\nN = m · g\nN = "+f(m)+" · "+f(g)+"\nN = "+f(N)+" N";}
   break;
  case 3:
   if(val(d,"mu",mu)&&val(d,"N",N))return "ATRITO\nFat = μ · N\nFat = "+f(mu)+" · "+f(N)+"\nFat = "+f(mu*N)+" N";
   if(val(d,"mu",mu)&&val(d,"m",m)){if(!val(d,"g",g))g=9.8;N=m*g;return "ATRITO\n1) N = m · g\nN = "+f(N)+" N\n2) Fat = μ · N\nFat = "+f(mu*N)+" N";}
   break;
  case 4:
   if(val(d,"k",k)&&val(d,"x",x))return "FORCA ELASTICA\nFe = k · x\nFe = "+f(k)+" · "+f(x)+"\nFe = "+f(k*x)+" N";
   break;
  case 5:
   if(val(d,"vi",vi)&&val(d,"a",a)&&val(d,"t",tm)){vf=vi+a*tm;return "CINEMATICA\nvf = vi + a · t\nvf = "+f(vi)+" + "+f(a)+" · "+f(tm)+"\nvf = "+f(vf)+" m/s";}
   if(val(d,"vf",vf)&&val(d,"vi",vi)&&val(d,"t",tm)&&tm!=0){a=(vf-vi)/tm;return "CINEMATICA\na = (vf - vi) / t\na = ("+f(vf)+" - "+f(vi)+") / "+f(tm)+"\na = "+f(a)+" m/s²";}
   if(val(d,"d",di)&&val(d,"t",tm)&&tm!=0){v=di/tm;return "CINEMATICA\nv = d / t\nv = "+f(di)+" / "+f(tm)+"\nv = "+f(v)+" m/s";}
   if(val(d,"v",v)&&val(d,"t",tm)){di=v*tm;return "CINEMATICA\nd = v · t\nd = "+f(v)+" · "+f(tm)+"\nd = "+f(di)+" m";}
   break;
  case 6:
   if(val(d,"m",m)&&val(d,"g",g)&&val(d,"h",h))return "ENERGIA POTENCIAL\nEp = m · g · h\nEp = "+f(m)+" · "+f(g)+" · "+f(h)+"\nEp = "+f(m*g*h)+" J";
   if(val(d,"m",m)&&val(d,"v",v))return "ENERGIA CINETICA\nEc = m · v² / 2\nEc = "+f(m)+" · "+f(v)+"² / 2\nEc = "+f(0.5*m*v*v)+" J";
   if(val(d,"F",F)&&val(d,"d",di)){double rad=theta*3.14159265358979323846/180.0; if(!val(d,"theta",theta))theta=0;rad=theta*3.14159265358979323846/180.0;return "TRABALHO\nW = F · d · cos(theta)\nW = "+f(F)+" · "+f(di)+" · cos("+f(theta)+")\nW = "+f(F*di*cos(rad))+" J";}
   break;
  case 7:
   if(val(d,"m",m)&&val(d,"V",V)&&V!=0)return "DENSIDADE\nrho = m / V\nrho = "+f(m)+" / "+f(V)+"\nrho = "+f(m/V)+" kg/m³";
   break;
 }
 return "DADOS INSUFICIENTES\nEscolha os dados conhecidos e tente novamente.";
}
String autoSolve(const String&d){
 for(uint8_t t=0;t<8;t++){String r=solve(t,d);if(r.indexOf("DADOS INSUFICIENTES")<0)return r;}
 return "Nao foi possivel identificar\numa solucao com os dados informados.";
}
}
