#pragma once
#include <Arduino.h>
namespace MathEngine {
struct Result { bool ok; double value; String text; };
Result evaluate(const String& expression, double x=0.0);
Result solveLinear(double a,double b);
Result solveQuadratic(double a,double b,double c);
Result solveEquation(const String& equation);
Result solveExponential(const String& equation);
String describeExpression(const String& expression);
}
