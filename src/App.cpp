// EXATA — interface reconstruida do zero para o M5Stack Cardputer ADV.
// A organizacao visual e a navegacao deste arquivo nao reutilizam a interface antiga.
#include "App.h"
#include "MathEngine.h"
#include "Physics.h"
#include "Chemistry.h"
#include "SDManager.h"
#include "WifiManager.h"
#include <M5Cardputer.h>
#include <SD.h>
#include <math.h>
#include <vector>

namespace {
constexpr int SW = 240;
constexpr int SH = 135;
constexpr uint16_t C_BG = 0x0841;
constexpr uint16_t C_PANEL = 0x10A3;
constexpr uint16_t C_LINE = 0x2945;
constexpr uint16_t C_TEXT = 0xFFFF;
constexpr uint16_t C_MUTED = 0x9CF3;
constexpr uint16_t C_CYAN = 0x05FF;
constexpr uint16_t C_LIME = 0xBFE0;
constexpr uint16_t C_GOLD = 0xFE60;
constexpr uint16_t C_PINK = 0xF91F;
constexpr uint16_t C_BLUE = 0x5DFF;

// Each section is a destination in its own right; the carousel is the home screen.
enum class Page : uint8_t { HOME, MENU, CALC, EQUATION, GRAPH, PHYSICS, PHYSICS_RESULT,
    PERIODIC, ELEMENT, BIOLOGY, TABLES, STUDY, LIBRARY, FILE_VIEW, WIFI, SETTINGS, ABOUT };
struct Tile { const char* title; const char* subtitle; const char* glyph; uint16_t color; Page page; };
const Tile homeTiles[] = {
    {"Calculadora", "Expressoes", "fx", C_CYAN, Page::CALC},
    {"Equacoes", "Resolver x", "=x", C_GOLD, Page::EQUATION},
    {"Graficos", "Funcoes", "f(x)", C_PINK, Page::GRAPH},
    {"Fisica", "Formulas e passos", "F", C_BLUE, Page::PHYSICS},
    {"Quimica", "Tabela periodica", "118", C_LIME, Page::PERIODIC},
    {"Biologia", "Conteudo visual", "BIO", C_PINK, Page::BIOLOGY},
    {"Tabelas", "SI e constantes", "SI", C_GOLD, Page::TABLES},
    {"Estudar", "Revisao rapida", "Q?", C_CYAN, Page::STUDY},
    {"Biblioteca", "Cartao microSD", "SD", C_BLUE, Page::LIBRARY},
    {"Enviar arquivos", "Wi-Fi local", "Wi", C_LIME, Page::WIFI},
    {"Configuracoes", "Tela e atalhos", "CFG", C_GOLD, Page::SETTINGS},
    {"Sobre a EXATA", "Ajuda e teclas", "i", C_CYAN, Page::ABOUT}
};
constexpr int HOME_COUNT = sizeof(homeTiles) / sizeof(homeTiles[0]);

const char* mathItems[] = {"Calculadora", "Equacoes", "Graficos", "Voltar"};
const char* physicsItems[] = {"Forca resultante", "Forca peso", "Forca normal", "Atrito", "Forca elastica", "Cinematica", "Energia e trabalho", "Densidade", "Voltar"};
const char* biologyItems[] = {"Fotossintese", "Respiracao celular", "Celula e organelas", "DNA e RNA", "Mitose", "Voltar"};
const char* tableItems[] = {"Constantes", "Prefixos SI", "Unidades basicas", "Conversoes comuns", "Voltar"};
const char* studyItems[] = {"Metodo de estudo", "Pergunta rapida", "Pausa focada", "Voltar"};

Page page = Page::HOME;
Page parentPage = Page::HOME;
int homeIndex = 0;
int menuIndex = 0;
int menuScroll = 0;
int elementZ = 1;
int physicsChoice = 0;
int bioChoice = 0;
int tableChoice = 0;
int studyChoice = 0;
String expression;
String output;
String currentPath = "/";
String selectedFile;
String fileText;
int fileOffset = 0;
bool editMode = false;
bool radians = true;
bool showSteps = true;
int decimals = 4;

void text(const String& s, int x, int y, uint16_t color=C_TEXT) {
    M5Cardputer.Display.setTextColor(color, C_BG);
    M5Cardputer.Display.drawString(s, x, y);
}
void clearScreen() {
    M5Cardputer.Display.fillScreen(C_BG);
    M5Cardputer.Display.setTextSize(1);
}
void line(int y, uint16_t c=C_LINE) { M5Cardputer.Display.drawFastHLine(7, y, SW-14, c); }
void header(const String& title, uint16_t accent=C_CYAN) {
    clearScreen();
    M5Cardputer.Display.fillRect(0, 0, SW, 17, C_PANEL);
    M5Cardputer.Display.fillRect(0, 16, 4, SH-16, accent);
    text("EXATA", 8, 3, accent);
    text(title, 51, 3, C_TEXT);
    text(SDManager::ready() ? "SD+" : "SD-", 207, 3, SDManager::ready() ? C_LIME : C_GOLD);
    line(18);
}
void footer(const String& s="</> mover   ENTER abrir   DEL voltar") {
    M5Cardputer.Display.fillRect(0, 122, SW, 13, C_PANEL);
    text(s, 7, 124, C_MUTED);
}
void panel(int x, int y, int w, int h, uint16_t color=C_PANEL, uint16_t edge=C_LINE) {
    M5Cardputer.Display.fillRoundRect(x,y,w,h,6,color);
    M5Cardputer.Display.drawRoundRect(x,y,w,h,6,edge);
}
void drawGlyph(const String& g, int x, int y, uint16_t color) {
    // Compact, original vector marks rather than bitmap assets.
    M5Cardputer.Display.drawRoundRect(x-18,y-18,36,36,7,color);
    M5Cardputer.Display.drawFastHLine(x-11,y+10,22,color);
    M5Cardputer.Display.drawFastVLine(x,y-10,20,color);
    M5Cardputer.Display.setTextColor(color,C_PANEL);
    M5Cardputer.Display.drawCenterString(g,x,y-5,1);
}
void drawHome() {
    header("CENTRAL DE ESTUDOS", C_CYAN);
    const Tile& t = homeTiles[homeIndex];
    // Neighbor previews establish a carousel, not the previous three-card layout.
    int left = (homeIndex + HOME_COUNT - 1) % HOME_COUNT;
    int right = (homeIndex + 1) % HOME_COUNT;
    panel(8,37,49,65,C_PANEL,C_LINE);
    text(homeTiles[left].glyph, 18, 49, homeTiles[left].color);
    text("<", 26, 76, C_MUTED);
    panel(183,37,49,65,C_PANEL,C_LINE);
    text(homeTiles[right].glyph, 194, 49, homeTiles[right].color);
    text(">", 203, 76, C_MUTED);
    panel(66,25,108,88,C_PANEL,t.color);
    M5Cardputer.Display.fillRoundRect(70,29,100,80,5,C_BG);
    drawGlyph(t.glyph,120,58,t.color);
    M5Cardputer.Display.setTextColor(C_TEXT,C_BG);
    M5Cardputer.Display.drawCenterString(t.title,120,81,1);
    M5Cardputer.Display.setTextColor(C_MUTED,C_BG);
    M5Cardputer.Display.drawCenterString(t.subtitle,120,96,1);
    text(String(homeIndex+1)+"/"+String(HOME_COUNT),194,108,C_MUTED);
    footer(" , / ou A/D trocar   ENTER abrir   1-9 atalho");
}
void drawList(const String& title, const char* const* items, int count, uint16_t accent=C_CYAN) {
    header(title,accent);
    int visible = 6;
    if (menuIndex < menuScroll) menuScroll = menuIndex;
    if (menuIndex >= menuScroll+visible) menuScroll = menuIndex-visible+1;
    for (int i=menuScroll; i<count && i<menuScroll+visible; ++i) {
        int y=24+(i-menuScroll)*15;
        if (i==menuIndex) {
            M5Cardputer.Display.fillRoundRect(8,y-2,224,14,3,accent);
            text(String("> ")+items[i],13,y,C_BG);
        } else text(items[i],13,y,C_TEXT);
    }
    text(String(menuIndex+1)+"/"+String(count),190,108,C_MUTED);
    footer("W/S mover   ENTER abrir   DEL voltar");
}
void drawInput(const String& title, const String& hint, uint16_t accent=C_CYAN) {
    header(title,accent);
    text(hint,9,25,C_MUTED);
    panel(8,43,224,26,C_BG,accent);
    String shown=expression;
    if(shown.length()>34) shown=shown.substring(shown.length()-34);
    text(shown+"_",13,52,C_TEXT);
    int p=0; int y=76;
    while(p<(int)output.length() && y<119) {
        int e=output.indexOf('\n',p); if(e<0)e=output.length();
        String row=output.substring(p,e); if(row.length()>37)row=row.substring(0,37);
        text(row,y==76?9:9,y,(y==76?C_LIME:C_TEXT));
        p=e+1; y+=12;
    }
    footer("Digite   ENTER calcular   DEL apagar/voltar");
}
void drawGraph() {
    header("GRAFICOS",C_PINK);
    text("y = "+(expression.length()?expression:String("x^2")),9,23,C_TEXT);
    int cx=120, cy=75, scale=13;
    for(int i=0;i<9;i++) {
        int x=14+i*26; M5Cardputer.Display.drawFastVLine(x,35,76,0x18E3);
    }
    for(int i=0;i<4;i++) { int y=39+i*22; M5Cardputer.Display.drawFastHLine(14,y,212,0x18E3); }
    M5Cardputer.Display.drawFastHLine(14,cy,212,C_MUTED);
    M5Cardputer.Display.drawFastVLine(cx,35,76,C_MUTED);
    // Evaluate a sampled function with the project's expression evaluator.
    int px=0,py=0; bool have=false;
    for(int sx=14;sx<226;sx+=2) {
        double x=(sx-cx)/(double)scale;
        auto r=MathEngine::evaluate(expression.length()?expression:String("x^2"),x);
        if(!r.ok || !isfinite(r.value) || fabs(r.value)>4.2) { have=false; continue; }
        int sy=cy-(int)lround(r.value*scale);
        if(have) M5Cardputer.Display.drawLine(px,py,sx,sy,C_PINK);
        px=sx; py=sy; have=true;
    }
    footer(editMode ? "Digite funcao   ENTER terminar edicao   DEL apagar" : "ENTER editar funcao   DEL voltar");
}
void drawPeriodic() {
    header("TABELA PERIODICA",C_LIME);
    const Chemistry::Element* e=Chemistry::byAtomicNumber((uint8_t)elementZ);
    text("ELEMENTO SELECIONADO",9,24,C_MUTED);
    panel(9,40,66,64,C_PANEL,C_LIME);
    if(e) {
        M5Cardputer.Display.setTextColor(C_LIME,C_PANEL);
        M5Cardputer.Display.drawCenterString(String(e->symbol),42,52,4);
        M5Cardputer.Display.setTextColor(C_TEXT,C_PANEL);
        M5Cardputer.Display.drawCenterString(String(e->z),42,84,1);
        text(e->name,86,43,C_TEXT);
        text("Grupo: "+String(e->group),86,58,C_MUTED);
        text("Periodo: "+String(e->period),86,71,C_MUTED);
        String cat=e->category; if(cat.length()>21)cat=cat.substring(0,21);
        text(cat,86,84,C_GOLD);
        text(String("Camadas: ")+e->config,86,97,C_TEXT);
    }
    footer("A/D ou ,/. elemento   ENTER detalhes   DEL voltar");
}
void drawElement() {
    const Chemistry::Element* e=Chemistry::byAtomicNumber((uint8_t)elementZ);
    header("FICHA DO ELEMENTO",C_LIME);
    if(!e) { text("Elemento indisponivel",9,35,C_GOLD); return; }
    text(String(e->symbol)+" - "+e->name,9,27,C_LIME);
    text("Numero atomico: "+String(e->z),9,43);
    text("Grupo "+String(e->group)+" / Periodo "+String(e->period),9,56);
    text(String("Familia: ")+e->category,9,69,C_GOLD);
    text(String("Camadas: ")+e->config,9,82);
    String note=Chemistry::octetNote(*e); if(note.length()>35)note=note.substring(0,35);
    text(note,9,96,C_MUTED);
    footer("A/D mudar elemento   DEL voltar");
}
void drawBiology() {
    header("BIOLOGIA",C_PINK);
    const char* title=""; const char* body="";
    switch(bioChoice) {
        case 0:title="FOTOSSINTESE"; body="Luz + agua + CO2\nproduzem glicose e O2.\nOcorre nos cloroplastos."; break;
        case 1:title="RESPIRACAO CELULAR"; body="Glicose + O2 geram\nenergia (ATP), agua e CO2.\nPrincipalmente na mitocondria."; break;
        case 2:title="CELULA"; body="Membrana: controla trocas.\nNucleo: guarda o DNA.\nRibossomos: produzem proteinas."; break;
        case 3:title="DNA E RNA"; body="DNA armazena informacao.\nRNA participa da expressao\ndos genes e sintese proteica."; break;
        case 4:title="MITOSE"; body="Uma celula origina duas\ncelulas-filhas com o mesmo\nnumero de cromossomos."; break;
        default:title="BIOLOGIA"; body="Escolha um tema para revisar."; break;
    }
    text(String(bioChoice+1)+" / 5",9,25,C_MUTED);
    panel(8,41,224,65,C_PANEL,C_PINK);
    text(title,17,48,C_PINK);
    int y=64; String all=body; int p=0;
    while(p<(int)all.length() && y<100) { int e=all.indexOf('\n',p); if(e<0)e=all.length(); text(all.substring(p,e),17,y,C_TEXT); p=e+1; y+=12; }
    footer("A/D proximo tema   DEL voltar");
}
void drawTables() {
    header("TABELAS DE CONSULTA",C_GOLD);
    const char* title=""; const char* body="";
    switch(tableChoice) {
        case 0:title="CONSTANTES";body="g = 9,8 m/s2\nc = 3,0 x 10^8 m/s\nNA = 6,022 x 10^23 mol-1";break;
        case 1:title="PREFIXOS SI";body="k = 10^3   M = 10^6\nc = 10^-2  m = 10^-3\nµ = 10^-6  n = 10^-9";break;
        case 2:title="UNIDADES BASICAS";body="Comprimento: metro (m)\nMassa: quilograma (kg)\nTempo: segundo (s)";break;
        default:title="CONVERSOES";body="1 km = 1000 m\n1 h = 3600 s\n1 L = 1000 mL";break;
    }
    text(title,9,29,C_GOLD);
    int p=0,y=48; String s=body;
    while(p<(int)s.length()) { int e=s.indexOf('\n',p); if(e<0)e=s.length(); text(s.substring(p,e),12,y,C_TEXT); y+=16;p=e+1; }
    footer("A/D proxima tabela   DEL voltar");
}
void drawStudy() {
    header("ESTUDAR",C_CYAN);
    if(studyChoice==0) {
        text("CICLO DE ESTUDO",9,28,C_CYAN);
        text("1. Defina uma meta pequena.",9,47);
        text("2. Estude sem notificacoes.",9,62);
        text("3. Resolva uma questao.",9,77);
        text("4. Revise o que errou.",9,92);
    } else if(studyChoice==1) {
        text("PERGUNTA RAPIDA",9,28,C_CYAN);
        text("Qual organela produz ATP",9,48);
        text("na respiracao celular?",9,61);
        text("Resposta: mitocondria",9,84,C_LIME);
    } else {
        text("PAUSA FOCADA",9,28,C_CYAN);
        text("Levante, beba agua e",9,48);
        text("descanse os olhos por",9,61);
        text("alguns minutos.",9,74);
    }
    footer("A/D mudar cartao   DEL voltar");
}
void drawPhysics() {
    drawList("ASSISTENTE DE FISICA",physicsItems,9,C_BLUE);
}
void drawPhysicsResult() {
    header("FISICA / RESULTADO",C_BLUE);
    if(editMode) { text("Dados: m=2,a=3 (exemplo)",9,22,C_MUTED); panel(8,35,224,22,C_BG,C_BLUE); text(expression+"_",12,41,C_TEXT); }
    int p=0,y=editMode?63:26;
    while(p<(int)output.length() && y<118) { int e=output.indexOf('\n',p); if(e<0)e=output.length(); String row=output.substring(p,e); if(row.length()>37)row=row.substring(0,37); text(row,9,y,y==26?C_LIME:C_TEXT); y+=12;p=e+1; }
    footer("ENTER novo calculo   DEL voltar");
}
void drawLibrary() {
    header("BIBLIOTECA SD",C_BLUE);
    text(currentPath,9,24,C_MUTED);
    String listing=SDManager::list(currentPath);
    int p=0,idx=0,y=41;
    while(p<(int)listing.length() && idx<5) {
        int e=listing.indexOf('\n',p); if(e<0)e=listing.length();
        String row=listing.substring(p,e);
        if(idx==menuIndex) { M5Cardputer.Display.fillRoundRect(8,y-2,224,13,3,C_BLUE); text("> "+row,12,y,C_BG); }
        else text(row,12,y,C_TEXT);
        p=e+1; y+=14; idx++;
    }
    if(listing=="SD nao disponivel") text("Verifique cartao e pinos SD.",9,81,C_GOLD);
    footer("W/S selecionar   ENTER abrir   DEL voltar");
}
void drawFile() {
    header(selectedFile,C_BLUE);
    int p=fileOffset;
    int y=23;
    for(int i=0;i<7 && p<(int)fileText.length();i++) {
        int e=fileText.indexOf('\n',p); if(e<0)e=fileText.length();
        String row=fileText.substring(p,e); if(row.length()>37)row=row.substring(0,37);
        text(row,8,y,C_TEXT); y+=13; p=e+1;
    }
    footer("W/S rolar texto   DEL voltar");
}
void drawWifi() {
    header("TRANSFERENCIA WI-FI",C_LIME);
    text("Rede local de arquivos",9,27,C_LIME);
    text("SSID: Cardputer-Estudos",9,46,C_TEXT);
    text("Senha: EXATA2026",9,60,C_TEXT);
    text("Endereco: 192.168.4.1",9,74,C_TEXT);
    text("Destino: /EXATA_SD",9,88,C_MUTED);
    text(WifiManager::running()?"Servidor ativo":"Servidor inicializando",9,103,WifiManager::running()?C_LIME:C_GOLD);
    footer("DEL voltar");
}
void drawSettings() {
    header("CONFIGURACOES",C_GOLD);
    text(String("Angulos: ")+(radians?"radianos":"graus"),10,32,C_TEXT);
    text(String("Casas decimais: ")+String(decimals),10,49,C_TEXT);
    text(String("Passo a passo: ")+(showSteps?"ligado":"desligado"),10,66,C_TEXT);
    text("ENTER alterna a opcao",10,88,C_MUTED);
    footer("W/S escolher   ENTER alterar   DEL voltar");
}
void drawAbout() {
    header("SOBRE A EXATA",C_CYAN);
    text("Assistente portatil de estudos",9,28,C_TEXT);
    text("Matematica, fisica, quimica",9,45,C_CYAN);
    text("biologia e biblioteca SD.",9,58,C_CYAN);
    text("Navegacao sem Fn: , / ou A/D",9,76,C_TEXT);
    text("ENTER abre; DEL volta.",9,89,C_TEXT);
    footer("EXATA / projeto educacional");
}
void render() {
    switch(page) {
        case Page::HOME: drawHome(); break;
        case Page::MENU: drawList("MATEMATICA",mathItems,4,C_CYAN); break;
        case Page::CALC: drawInput("CALCULADORA","Ex.: 2+3*4  2^5  sqrt(81)  sin(pi/2)"); break;
        case Page::EQUATION: drawInput("RESOLVER EQUACAO","Ex.: 2*x+4=10 ou x^2-5*x+6=0",C_GOLD); break;
        case Page::GRAPH: drawGraph(); break;
        case Page::PHYSICS: drawPhysics(); break;
        case Page::PHYSICS_RESULT: drawPhysicsResult(); break;
        case Page::PERIODIC: drawPeriodic(); break;
        case Page::ELEMENT: drawElement(); break;
        case Page::BIOLOGY: drawBiology(); break;
        case Page::TABLES: drawTables(); break;
        case Page::STUDY: drawStudy(); break;
        case Page::LIBRARY: drawLibrary(); break;
        case Page::FILE_VIEW: drawFile(); break;
        case Page::WIFI: drawWifi(); break;
        case Page::SETTINGS: drawSettings(); break;
        case Page::ABOUT: drawAbout(); break;
    }
}
void resetInput() { expression=""; output=""; editMode=false; }
void goHome() { page=Page::HOME; resetInput(); menuIndex=0; menuScroll=0; }
void openHomeTile() {
    Page target=homeTiles[homeIndex].page;
    parentPage=Page::HOME;
    page=target; menuIndex=0; menuScroll=0; output="";
    if(target==Page::CALC || target==Page::EQUATION) { expression=""; editMode=true; }
    if(target==Page::GRAPH) { expression="x^2"; editMode=false; }
    if(target==Page::PERIODIC) elementZ=1;
    if(target==Page::LIBRARY) { currentPath="/EXATA_SD"; menuIndex=0; }
    if(target==Page::WIFI) { WifiManager::begin(); }
}
void openSelected() {
    if(page==Page::HOME) { openHomeTile(); return; }
    if(page==Page::MENU) {
        if(menuIndex==0) { page=Page::CALC; expression=""; editMode=true; }
        else if(menuIndex==1) { page=Page::EQUATION; expression=""; editMode=true; }
        else if(menuIndex==2) { page=Page::GRAPH; expression="x^2"; editMode=false; }
        else goHome(); return;
    }
    if(page==Page::CALC) {
        auto r=MathEngine::evaluate(expression);
        output=r.ok ? String("Resultado: ")+String(r.value,decimals) : r.text;
        editMode=false; return;
    }
    if(page==Page::EQUATION) {
        auto r=MathEngine::solveEquation(expression);
        output=r.text; editMode=false; return;
    }
    if(page==Page::GRAPH) { editMode=!editMode; return; }
    if(page==Page::PERIODIC) { page=Page::ELEMENT; return; }
    if(page==Page::PHYSICS) {
        if(menuIndex>=8) { goHome(); return; }
        physicsChoice=menuIndex;
        output=String(Physics::name((uint8_t)physicsChoice))+"\n"+Physics::formula((uint8_t)physicsChoice)+"\n\nDigite os dados no formato m=2,a=3\nENTER calcula.";
        expression=""; editMode=true; page=Page::PHYSICS_RESULT; return;
    }
    if(page==Page::BIOLOGY) { bioChoice=(bioChoice+1)%5; return; }
    if(page==Page::TABLES) { tableChoice=(tableChoice+1)%4; return; }
    if(page==Page::STUDY) { studyChoice=(studyChoice+1)%3; return; }
    if(page==Page::LIBRARY) {
        String listing=SDManager::list(currentPath); int p=0,idx=0; String row="";
        while(p<(int)listing.length()) { int e=listing.indexOf('\n',p); if(e<0)e=listing.length(); if(idx==menuIndex){row=listing.substring(p,e);break;} p=e+1;idx++; }
        if(row.length()==0 || row=="(vazio)" || row=="SD nao disponivel") return;
        bool dir=row.startsWith("[D]"); String name=row.substring(4); String full=SDManager::normalize(currentPath,name);
        if(dir) { currentPath=full; menuIndex=0; }
        else { selectedFile=name; fileText=SDManager::readText(full); fileOffset=0; page=Page::FILE_VIEW; }
        return;
    }
    if(page==Page::SETTINGS) {
        if(menuIndex==0) radians=!radians;
        else if(menuIndex==1) decimals=(decimals+1)%7;
        else showSteps=!showSteps;
        return;
    }
}
void back() {
    if(page==Page::HOME) return;
    if(page==Page::ELEMENT) { page=Page::PERIODIC; return; }
    if(page==Page::FILE_VIEW) { page=Page::LIBRARY; return; }
    if(page==Page::PHYSICS_RESULT) { page=Page::PHYSICS; expression=""; editMode=false; return; }
    if(page==Page::MENU || parentPage==Page::HOME) { goHome(); return; }
    goHome();
}
void moveHorizontal(int d) {
    if(page==Page::HOME) { homeIndex=(homeIndex+d+HOME_COUNT)%HOME_COUNT; return; }
    if(page==Page::PERIODIC || page==Page::ELEMENT) { elementZ+=d; if(elementZ<1)elementZ=118; if(elementZ>118)elementZ=1; return; }
    if(page==Page::BIOLOGY) { bioChoice=(bioChoice+d+5)%5; return; }
    if(page==Page::TABLES) { tableChoice=(tableChoice+d+4)%4; return; }
    if(page==Page::STUDY) { studyChoice=(studyChoice+d+3)%3; return; }
}
void moveVertical(int d) {
    if (page==Page::FILE_VIEW) {
        for (int n=0; n<7; ++n) {
            if (d<0)
