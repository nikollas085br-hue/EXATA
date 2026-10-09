#include <M5Cardputer.h>
#include <cmath>
#include <vector>

// EXATA - interface inicial para M5Stack Cardputer ADV
// Teclas: M menu/voltar; W/A/S/D mover gráfico; + e - zoom; C cos; T tan; R raiz; P pi.

static constexpr int SCREEN_W = 240;
static constexpr int SCREEN_H = 135;
static constexpr uint16_t C_BG = 0x0841;
static constexpr uint16_t C_PANEL = 0x18E3;
static constexpr uint16_t C_ACCENT = 0x07FF;
static constexpr uint16_t C_TEXT = 0xFFFF;
static constexpr uint16_t C_MUTED = 0xBDF7;
static constexpr uint16_t C_ROOT = 0xF800;

enum ScreenMode { HOME, GRAPH, SHORTCUTS, HELP };
ScreenMode screenMode = HOME;

double xMin = 0.1, xMax = 6.0;
double yMin = -10.0, yMax = 20.0;
String lastShortcut = "Nenhum atalho selecionado";

// Função de demonstração recebida no código original.
// Este exemplo ainda não é um parser de expressões matemáticas arbitrárias.
double evaluateF(double x) {
  if (std::abs(x) < 1e-12) return NAN;
  const double term1 = std::pow(2.0, (x * x - 3.0 * x) / x);
  const double argLog = std::pow(4.0, x) - std::pow(2.0, x + 2.0) + 8.0;
  if (argLog <= 0.0) return NAN;
  const double term2 = std::log2(argLog);
  const double term3 = std::pow(2.0, x + 1.0) - 4.0;
  return term1 * term2 - term3;
}

double bisection(double a, double b, double tol = 1e-4) {
  double fa = evaluateF(a);
  double fb = evaluateF(b);
  if (!std::isfinite(fa) || !std::isfinite(fb) || fa * fb > 0) return NAN;
  double c = a;
  for (int i = 0; i < 80 && (b - a) >= tol; ++i) {
    c = (a + b) / 2.0;
    const double fc = evaluateF(c);
    if (!std::isfinite(fc)) return NAN;
    if (std::abs(fc) < 1e-10) return c;
    if (fa * fc < 0) { b = c; fb = fc; }
    else { a = c; fa = fc; }
  }
  return c;
}

std::vector<double> findRoots(double lo, double hi, double step = 0.1) {
  std::vector<double> roots;
  for (double x = lo; x < hi; x += step) {
    const double y1 = evaluateF(x), y2 = evaluateF(x + step);
    if (!std::isfinite(y1) || !std::isfinite(y2) || y1 * y2 > 0) continue;
    const double root = bisection(x, x + step);
    if (!std::isfinite(root)) continue;
    bool unique = true;
    for (double old : roots) if (std::abs(old - root) < 1e-3) unique = false;
    if (unique) roots.push_back(root);
  }
  return roots;
}

int mapX(double x) { return int((x - xMin) / (xMax - xMin) * (SCREEN_W - 1)); }
int mapY(double y) { return int((SCREEN_H - 1) - (y - yMin) / (yMax - yMin) * (SCREEN_H - 1)); }

void header(const char* title) {
  M5.Display.fillRect(0, 0, SCREEN_W, 18, C_PANEL);
  M5.Display.setTextColor(C_ACCENT, C_PANEL);
  M5.Display.setTextSize(1);
  M5.Display.setCursor(6, 5);
  M5.Display.print("EXATA");
  M5.Display.setTextColor(C_TEXT, C_PANEL);
  M5.Display.setCursor(58, 5);
  M5.Display.print(title);
}

void drawHome() {
  M5.Display.fillScreen(C_BG);
  header("CALCULADORA CIENTIFICA");
  M5.Display.setTextColor(C_ACCENT, C_BG);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(12, 29); M5.Display.print("EXATA");
  M5.Display.setTextSize(1);
  M5.Display.setTextColor(C_TEXT, C_BG);
  M5.Display.setCursor(13, 51); M5.Display.print("Estudos • Matematica");
  M5.Display.drawRoundRect(8, 68, 108, 24, 4, C_ACCENT);
  M5.Display.setCursor(18, 77); M5.Display.print("G  Grafico");
  M5.Display.drawRoundRect(124, 68, 108, 24, 4, C_ACCENT);
  M5.Display.setCursor(133, 77); M5.Display.print("M  Atalhos");
  M5.Display.setTextColor(C_MUTED, C_BG);
  M5.Display.setCursor(8, 105); M5.Display.print("H: ajuda   G: grafico   M: menu");
  M5.Display.setCursor(8, 119); M5.Display.print("Setas WASD: mover grafico | +/- zoom");
}

void drawGraph() {
  M5.Display.fillScreen(C_BG);
  auto roots = findRoots(xMin, xMax);
  // Eixos, quando dentro da janela.
  int zeroY = mapY(0), zeroX = mapX(0);
  if (zeroY >= 18 && zeroY < SCREEN_H) M5.Display.drawFastHLine(0, zeroY, SCREEN_W, 0x4208);
  if (zeroX >= 0 && zeroX < SCREEN_W) M5.Display.drawFastVLine(zeroX, 18, SCREEN_H - 18, 0x4208);
  int prevX = -1, prevY = -1;
  for (int px = 0; px < SCREEN_W; ++px) {
    const double x = xMin + double(px) / double(SCREEN_W - 1) * (xMax - xMin);
    const double y = evaluateF(x);
    if (!std::isfinite(y)) { prevX = -1; continue; }
    const int py = mapY(y);
    if (py >= 18 && py < SCREEN_H) {
      if (prevX >= 0 && std::abs(py - prevY) < SCREEN_H / 2) M5.Display.drawLine(prevX, prevY, px, py, C_ACCENT);
      else M5.Display.drawPixel(px, py, C_ACCENT);
      prevX = px; prevY = py;
    } else prevX = -1;
  }
  for (double r : roots) {
    int rx = mapX(r), ry = mapY(evaluateF(r));
    if (rx >= 0 && rx < SCREEN_W && ry >= 18 && ry < SCREEN_H) M5.Display.fillCircle(rx, ry, 3, C_ROOT);
  }
  header("GRAFICO f(x)");
  M5.Display.setTextColor(C_TEXT, C_BG);
  M5.Display.setCursor(5, 21);
  if (!roots.empty()) M5.Display.printf("Raiz aprox: %.3f", roots.front());
  else M5.Display.print("Nenhuma raiz detectada nesta janela");
  M5.Display.setTextColor(C_MUTED, C_BG);
  M5.Display.setCursor(5, 124); M5.Display.print("WASD mover  +/- zoom  M menu");
}

void drawShortcuts() {
  M5.Display.fillScreen(C_BG);
  header("MENU DE ATALHOS");
  M5.Display.setTextColor(C_TEXT, C_BG);
  M5.Display.setCursor(8, 25); M5.Display.print("C  cos(x)       T  tan(x)");
  M5.Display.setCursor(8, 40); M5.Display.print("S  sin(x)       R  raiz quadrada");
  M5.Display.setCursor(8, 55); M5.Display.print("P  constante pi  L  log10(x)");
  M5.Display.setCursor(8, 70); M5.Display.print("N  ln(x)         E  numero e");
  M5.Display.setCursor(8, 91); M5.Display.setTextColor(C_ACCENT, C_BG); M5.Display.print("Atalho: "); M5.Display.print(lastShortcut);
  M5.Display.setTextColor(C_MUTED, C_BG);
  M5.Display.setCursor(8, 119); M5.Display.print("M volta | H ajuda | G abre grafico");
}

void drawHelp() {
  M5.Display.fillScreen(C_BG); header("AJUDA RAPIDA");
  M5.Display.setTextColor(C_TEXT, C_BG);
  M5.Display.setCursor(8, 27); M5.Display.print("M  menu de atalhos / voltar");
  M5.Display.setCursor(8, 43); M5.Display.print("G  abrir grafico da funcao");
  M5.Display.setCursor(8, 59); M5.Display.print("W/A/S/D  deslocar a janela");
  M5.Display.setCursor(8, 75); M5.Display.print("+/-  aproximar / afastar");
  M5.Display.setCursor(8, 91); M5.Display.print("C/T/S/R/P/L/N/E  atalhos matematicos");
  M5.Display.setTextColor(C_MUTED, C_BG);
  M5.Display.setCursor(8, 119); M5.Display.print("Teclas inserem atalhos informativos.");
}

void redraw() {
  if (screenMode == HOME) drawHome();
  else if (screenMode == GRAPH) drawGraph();
  else if (screenMode == SHORTCUTS) drawShortcuts();
  else drawHelp();
}

void handleKey(char key) {
  if (key >= 'a' && key <= 'z') key = char(key - 'a' + 'A');
  if (key == 'M') { screenMode = (screenMode == SHORTCUTS) ? HOME : SHORTCUTS; redraw(); return; }
  if (key == 'H') { screenMode = HELP; redraw(); return; }
  if (key == 'G') { screenMode = GRAPH; redraw(); return; }
  if (key == 'C') lastShortcut = "cos(x)";
  else if (key == 'T') lastShortcut = "tan(x)";
  else if (key == 'S') lastShortcut = "sin(x)";
  else if (key == 'R') lastShortcut = "sqrt(x) - raiz quadrada";
  else if (key == 'P') lastShortcut = "pi = 3.14159265...";
  else if (key == 'L') lastShortcut = "log10(x)";
  else if (key == 'N') lastShortcut = "ln(x)";
  else if (key == 'E') lastShortcut = "e = 2.71828182...";
  else if (screenMode == GRAPH && (key == 'W' || key == 'A' || key == 'S' || key == 'D' || key == '+' || key == '-')) {
    if (key == 'W' || key == 'S') { double dy = (yMax - yMin) * 0.1; if (key == 'W') { yMin += dy; yMax += dy; } else { yMin -= dy; yMax -= dy; } }
    if (key == 'A' || key == 'D') { double dx = (xMax - xMin) * 0.1; if (key == 'A') { xMin -= dx; xMax -= dx; } else { xMin += dx; xMax += dx; } }
    if (key == '+' || key == '-') { double dx = (xMax - xMin) * 0.2; if (key == '+') { xMin += dx; xMax -= dx; } else { xMin -= dx; xMax += dx; } }
  }
  redraw();
}

void setup() {
  auto cfg = M5.config();
  M5Cardputer.begin(cfg, true);
  M5.Display.setRotation(1);
  M5.Display.setTextSize(1);
  redraw();
}

void loop() {
  M5Cardputer.update();
  if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
    auto keys = M5Cardputer.Keyboard.keysState();
    for (auto key : keys.word) handleKey(key);
    if (keys.enter) handleKey('G');
  }
  delay(20);
}
