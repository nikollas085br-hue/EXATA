#include "WifiManager.h"
#include <WiFi.h>
#include <WebServer.h>
#include <SD.h>

namespace {
WebServer server(80);
bool active = false;
File uploadFile;
String uploadTarget = "/";

String urlDecode(String s) {
  s.replace("+", " ");
  for (int i = 0; i + 2 < (int)s.length(); ++i) {
    if (s[i] == '%') {
      char h1 = s[i + 1], h2 = s[i + 2];
      auto hex = [](char c)->int { if(c>='0'&&c<='9') return c-'0'; if(c>='A'&&c<='F') return c-'A'+10; if(c>='a'&&c<='f') return c-'a'+10; return -1; };
      int a = hex(h1), b = hex(h2);
      if (a >= 0 && b >= 0) { s.setCharAt(i, char((a << 4) | b)); s.remove(i + 1, 2); }
    }
  }
  return s;
}

String cleanPath(String p) {
  p = urlDecode(p);
  p.trim();
  if (!p.startsWith("/")) p = "/EXATA_SD/" + p;
  if (p == "/") p = "/EXATA_SD";
  while (p.indexOf("//") >= 0) p.replace("//", "/");
  if (p.indexOf("..") >= 0) return "/EXATA_SD";
  if (p != "/EXATA_SD" && !p.startsWith("/EXATA_SD/")) return "/EXATA_SD";
  return p;
}

String parentOf(String p) {
  p = cleanPath(p);
  if (p == "/") return "/";
  int i = p.lastIndexOf('/');
  if (i <= 0) return "/";
  return p.substring(0, i);
}

String baseName(String p) {
  p = cleanPath(p);
  int i = p.lastIndexOf('/');
  return i < 0 ? p : p.substring(i + 1);
}

String extension(String p) {
  int i = p.lastIndexOf('.');
  if (i < 0) return "";
  String e = p.substring(i + 1);
  e.toLowerCase();
  return e;
}

String jsonEscape(String s) {
  s.replace("\\", "\\\\");
  s.replace("\"", "\\\"");
  s.replace("\r", "\\r");
  s.replace("\n", "\\n");
  return s;
}

String contentType(const String &p) {
  String e = extension(p);
  if (e == "html" || e == "htm") return "text/html; charset=utf-8";
  if (e == "css") return "text/css; charset=utf-8";
  if (e == "js") return "application/javascript; charset=utf-8";
  if (e == "json") return "application/json; charset=utf-8";
  if (e == "txt" || e == "md" || e == "csv" || e == "log" || e == "ini" || e == "py" || e == "cpp" || e == "h" || e == "hpp") return "text/plain; charset=utf-8";
  if (e == "png") return "image/png";
  if (e == "jpg" || e == "jpeg") return "image/jpeg";
  if (e == "bmp") return "image/bmp";
  if (e == "gif") return "image/gif";
  if (e == "svg") return "image/svg+xml";
  if (e == "pdf") return "application/pdf";
  if (e == "ppt") return "application/vnd.ms-powerpoint";
  if (e == "pptx") return "application/vnd.openxmlformats-officedocument.presentationml.presentation";
  return "application/octet-stream";
}

bool isAllowedPath(const String &p) {
  return p == "/EXATA_SD" || (p.startsWith("/EXATA_SD/") && p.indexOf("..") < 0);
}

String listJson(const String &dirPath) {
  String p = cleanPath(dirPath);
  File dir = SD.open(p);
  if (!dir || !dir.isDirectory()) { if (dir) dir.close(); return "[]"; }
  String out = "[";
  bool first = true;
  File f;
  while ((f = dir.openNextFile())) {
    String name = String(f.name());
    String shortName = baseName(name);
    if (!first) out += ",";
    first = false;
    out += "{\"name\":\"" + jsonEscape(shortName) + "\",\"path\":\"" + jsonEscape(name) + "\",\"dir\":" + String(f.isDirectory() ? "true" : "false") + ",\"size\":" + String((unsigned long)f.size()) + ",\"ext\":\"" + jsonEscape(extension(name)) + "\"}";
    f.close();
  }
  dir.close();
  out += "]";
  return out;
}

const char PAGE[] PROGMEM = R"HTML(
<!doctype html><html lang="pt-BR"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Cardputer • Central de Estudos</title>
<style>
:root{color-scheme:dark;--bg:#0b0f14;--panel:#121922;--line:#263342;--accent:#22d3ee;--muted:#93a4b8}*{box-sizing:border-box}body{margin:0;background:var(--bg);font:15px system-ui,Arial;color:#eef5fb}header{padding:16px 18px;border-bottom:1px solid var(--line);position:sticky;top:0;background:#0b0f14ee;backdrop-filter:blur(8px);z-index:5}h1{font-size:20px;margin:0 0 4px}small,.muted{color:var(--muted)}main{max-width:1100px;margin:auto;padding:16px}.bar,.card{background:var(--panel);border:1px solid var(--line);border-radius:12px;padding:12px;margin-bottom:12px}.bar{display:flex;gap:8px;flex-wrap:wrap;align-items:center}.path{font-family:monospace;color:var(--accent);word-break:break-all}.grid{display:grid;grid-template-columns:repeat(auto-fill,minmax(220px,1fr));gap:10px}.item{background:#0f1620;border:1px solid var(--line);border-radius:10px;padding:10px;min-height:92px;display:flex;flex-direction:column;justify-content:space-between}.name{font-weight:650;word-break:break-word}.meta{font-size:12px;color:var(--muted);margin-top:5px}button,input{border:1px solid var(--line);background:#18222e;color:#fff;border-radius:8px;padding:9px}button{cursor:pointer}button:hover{border-color:var(--accent)}input[type=file]{max-width:100%}.preview{max-width:100%;max-height:65vh;display:block;margin:auto;border-radius:8px}.viewer{white-space:pre-wrap;overflow:auto;max-height:70vh;background:#090d12;border:1px solid var(--line);padding:14px;border-radius:8px;font:13px ui-monospace,monospace}.tag{display:inline-block;border:1px solid var(--line);border-radius:999px;padding:2px 7px;font-size:11px;color:var(--muted)}.hidden{display:none}.row{display:flex;gap:8px;flex-wrap:wrap}.danger{border-color:#6b3030}.success{border-color:#276149}.notice{color:#f5c76b}.thumb{width:100%;height:120px;object-fit:contain;background:#080c10;border-radius:7px;margin-bottom:7px}.actions{display:flex;gap:6px;flex-wrap:wrap;margin-top:8px}.actions button{font-size:12px;padding:6px}.drop{border:1px dashed #405269;text-align:center;padding:18px;border-radius:10px}
</style></head><body><header><h1>Cardputer • Central de Estudos</h1><small>Gerenciador completo do cartão SD pela rede local</small></header><main>
<div class="bar"><button onclick="go('/')">Início</button><button onclick="up()">⬆ Pasta anterior</button><button onclick="refresh()">↻ Atualizar</button><span class="path" id="path">/</span></div>
<div class="card"><b>Enviar arquivos</b><div class="muted">Destino atual: <span id="target">/</span></div><div class="row"><input id="files" type="file" multiple><button class="success" onclick="upload()">Enviar para esta pasta</button><button onclick="newFolder()">Nova pasta</button></div><div id="status" class="muted"></div></div>
<div id="list" class="grid"></div>
<div id="viewer" class="card hidden"><div class="row"><b id="vtitle">Visualizador</b><button onclick="closeViewer()">Fechar</button></div><div id="vbody" style="margin-top:10px"></div></div>
</main><script>
let current='/EXATA_SD';
const $=id=>document.getElementById(id);
function esc(s){return s.replace(/[&<>\"]/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;'}[c]))}
function fmt(n){if(n<1024)return n+' B';if(n<1048576)return (n/1024).toFixed(1)+' KB';return (n/1048576).toFixed(1)+' MB'}
function go(p){current=p;refresh()}
function up(){if(current!='/'){let a=current.split('/').filter(Boolean);a.pop();current='/'+a.join('/');if(current!='/'&&!current.endsWith('/')){}refresh()}}
async function refresh(){ $('path').textContent=current;$('target').textContent=current;$('status').textContent='';let r=await fetch('/api/list?path='+encodeURIComponent(current));let data=await r.json();let html='';if(current!='/')html+=`<div class="item"><div class="name">📁 ..</div><div class="meta">Pasta anterior</div><button onclick="up()">Abrir</button></div>`;for(const x of data){let icon=x.dir?'📁':iconFor(x.ext);let img=!x.dir&&['png','jpg','jpeg','bmp','gif','svg'].includes(x.ext)?`<img class="thumb" src="/file?path=${encodeURIComponent(x.path)}">`:'';html+=`<div class="item">${img}<div><div class="name">${icon} ${esc(x.name)}</div><div class="meta">${x.dir?'PASTA':fmt(x.size)+' • '+(x.ext||'arquivo').toUpperCase()}</div></div><div class="actions"><button onclick='openItem(${JSON.stringify(x)})'>Abrir</button><button onclick='renameItem(${JSON.stringify(x)})'>Renomear</button><button onclick='deleteItem(${JSON.stringify(x)})' class="danger">Excluir</button></div></div>`}$('list').innerHTML=html||'<div class="muted">Pasta vazia.</div>'}
function iconFor(e){if(['png','jpg','jpeg','bmp','gif','svg'].includes(e))return'🖼️';if(['txt','md','csv','log'].includes(e))return'📄';if(['py','cpp','h','hpp','ino'].includes(e))return'💻';if(['pdf'].includes(e))return'📕';if(['ppt','pptx'].includes(e))return'📊';return'📦'}
async function openItem(x){if(x.dir){go(x.path);return}let e=x.ext;if(['png','jpg','jpeg','bmp','gif','svg'].includes(e)){show(x.name,`<img class="preview" src="/file?path=${encodeURIComponent(x.path)}"><div class="actions"><a href="/download?path=${encodeURIComponent(x.path)}"><button>Baixar</button></a></div>`);return}if(['txt','md','csv','log','ini','json','py','cpp','h','hpp','ino','js','css'].includes(e)){let t=await (await fetch('/file?path='+encodeURIComponent(x.path))).text();show(x.name,`<pre class="viewer">${esc(t)}</pre><div class="actions"><a href="/download?path=${encodeURIComponent(x.path)}"><button>Baixar</button></a></div>`);return}if(e==='pdf'){show(x.name,`<iframe style="width:100%;height:70vh;border:0" src="/file?path=${encodeURIComponent(x.path)}"></iframe><p class="notice">A visualização depende do navegador. O arquivo continua armazenado no SD.</p>`);return}if(e==='ppt'||e==='pptx'){show(x.name,`<p>PowerPoint identificado.</p><p class="muted">O Cardputer não executa o PowerPoint. Você pode baixar o arquivo ou manter o material no SD para uso futuro.</p><a href="/download?path=${encodeURIComponent(x.path)}"><button>Baixar</button></a>`);return}show(x.name,`<p class="muted">Formato ${esc(e||'desconhecido')} armazenado no SD.</p><a href="/download?path=${encodeURIComponent(x.path)}"><button>Baixar</button></a>`)}
function show(t,b){$('viewer').classList.remove('hidden');$('vtitle').textContent=t;$('vbody').innerHTML=b;scrollTo(0,document.body.scrollHeight)}function closeViewer(){$('viewer').classList.add('hidden')}
async function upload(){let fs=$('files').files;if(!fs.length){$('status').textContent='Escolha pelo menos um arquivo.';return}let n=0;for(const f of fs){let fd=new FormData();fd.append('file',f,f.name);let r=await fetch('/upload?path='+encodeURIComponent(current),{method:'POST',body:fd});if(r.ok)n++}$('status').textContent=n+' arquivo(s) enviado(s) para '+current;refresh()}
async function newFolder(){let n=prompt('Nome da nova pasta:');if(!n)return;let r=await fetch('/mkdir?path='+encodeURIComponent(current+'/'+n),{method:'POST'});alert(r.ok?'Pasta criada.':'Não foi possível criar a pasta.');refresh()}
async function renameItem(x){let n=prompt('Novo nome (mantenha a extensão):',x.name);if(!n||n===x.name)return;let r=await fetch('/rename?from='+encodeURIComponent(x.path)+'&to='+encodeURIComponent((current==='/'?'':current+'/')+n),{method:'POST'});alert(await r.text());refresh()}
async function deleteItem(x){if(!confirm('Excluir '+x.name+'?'))return;let r=await fetch('/delete?path='+encodeURIComponent(x.path),{method:'POST'});alert(await r.text());refresh()}
refresh();
</script></body></html>
)HTML";

void page() { server.send_P(200, "text/html; charset=utf-8", PAGE); }

void apiList() {
  String p = cleanPath(server.arg("path"));
  server.send(200, "application/json; charset=utf-8", listJson(p));
}

void streamFile(bool download) {
  String p = cleanPath(server.arg("path"));
  if (!isAllowedPath(p) || !SD.exists(p)) { server.send(404, "text/plain", "Arquivo nao encontrado"); return; }
  File f = SD.open(p, FILE_READ);
  if (!f || f.isDirectory()) { if(f) f.close(); server.send(404, "text/plain", "Arquivo invalido"); return; }
  if (download) server.sendHeader("Content-Disposition", "attachment; filename=\"" + baseName(p) + "\"");
  server.streamFile(f, contentType(p));
  f.close();
}

void uploadHandler() {
  HTTPUpload &u = server.upload();
  if (u.status == UPLOAD_FILE_START) {
    String name = baseName(urlDecode(u.filename));
    if (!name.length() || name.indexOf("..") >= 0) return;
    uploadTarget = cleanPath(server.arg("path"));
    if (!SD.exists(uploadTarget)) SD.mkdir(uploadTarget);
    String dest = uploadTarget == "/" ? "/" + name : uploadTarget + "/" + name;
    if (SD.exists(dest)) SD.remove(dest);
    uploadFile = SD.open(dest, FILE_WRITE);
  } else if (u.status == UPLOAD_FILE_WRITE) {
    if (uploadFile) uploadFile.write(u.buf, u.currentSize);
  } else if (u.status == UPLOAD_FILE_END) {
    if (uploadFile) uploadFile.close();
  }
}

void finishUpload() { server.send(200, "text/plain; charset=utf-8", "Upload concluido"); }

void makeDir() {
  String p = cleanPath(server.arg("path"));
  if (!isAllowedPath(p) || SD.exists(p)) { server.send(400, "text/plain", "Pasta invalida ou ja existe"); return; }
  bool ok = SD.mkdir(p);
  server.send(ok ? 200 : 500, "text/plain", ok ? "Pasta criada" : "Falha ao criar pasta");
}

void renameItem() {
  String a = cleanPath(server.arg("from"));
  String b = cleanPath(server.arg("to"));
  if (!isAllowedPath(a) || !isAllowedPath(b) || !SD.exists(a) || SD.exists(b) || baseName(b).length() == 0) { server.send(400, "text/plain", "Nome/caminho invalido ou destino ja existe"); return; }
  String oldExt = extension(a), newExt = extension(b); oldExt.toLowerCase(); newExt.toLowerCase();
  if (oldExt != newExt) { server.send(400, "text/plain", "A funcao N apenas renomeia; mantenha a extensao do arquivo"); return; }
  bool ok = SD.rename(a,b);
  server.send(ok ? 200 : 500, "text/plain", ok ? "Renomeado" : "Falha ao renomear");
}

void deleteItem() {
  String p = cleanPath(server.arg("path"));
  if (p == "/" || !SD.exists(p)) { server.send(400, "text/plain", "Arquivo/pasta invalido"); return; }
  File f = SD.open(p);
  bool dir = f && f.isDirectory(); if(f) f.close();
  if (dir) { server.send(400, "text/plain", "Para seguranca, exclua os arquivos antes da pasta"); return; }
  bool ok = SD.remove(p);
  server.send(ok ? 200 : 500, "text/plain", ok ? "Excluido" : "Falha ao excluir");
}
}

namespace WifiManager {
void begin() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP("Cardputer-Estudos", "EXATA2026");
  server.on("/", HTTP_GET, page);
  server.on("/api/list", HTTP_GET, apiList);
  server.on("/file", HTTP_GET, [](){ streamFile(false); });
  server.on("/download", HTTP_GET, [](){ streamFile(true); });
  server.on("/upload", HTTP_POST, finishUpload, uploadHandler);
  server.on("/mkdir", HTTP_POST, makeDir);
  server.on("/rename", HTTP_POST, renameItem);
  server.on("/delete", HTTP_POST, deleteItem);
  server.begin();
  active = true;
}
void update() { if(active) server.handleClient(); }
bool running() { return active; }
}
