# EXATA — Arquitetura de Firmware Embarcado e Servidor Web Local para M5Cardputer

O **EXATA** é um sistema de firmware autônomo de baixo nível desenvolvido em C++ e estruturado através do ecossistema de compilação **PlatformIO**. Ele foi projetado especificamente para operar no hardware do **M5Stack Cardputer**, redefinindo as capacidades operacionais do microcontrolador ESP32-S3 integrado para transformá-lo em um nó de rede autônomo com capacidade de processamento de arquivos em tempo de execução e hospedagem de aplicações web estáticas.

---

## 1. Arquitetura de Software e Camada de Abstração de Hardware (HAL)

O núcleo do projeto opera na interseção entre o gerenciamento de sistemas operacionais em tempo real (FreeRTOS) e o controle direto de periféricos físicos através de bibliotecas otimizadas:

* **Gerenciamento de Núcleos (Dual-Core):** O ESP32-S3 possui múltiplos núcleos de processamento. O firmware gerencia o escalonamento de tarefas para isolar as rotinas de rede e manipulação de interrupções de hardware (como varredura matricial do teclado físico) de gargalos de I/O de arquivos.
* **Abstração de Periféricos (M5GFX e M5Unified):** A interface gráfica e a gestão de energia bypassam camadas intermediárias de sistema operacional pesadas, interagindo diretamente com os registradores do display integrado e gerenciador de barramento I2C, garantindo latência mínima na renderização de texto e menus.
* **Varredura Matricial de Teclado:** O mapeamento físico do teclado do Cardputer é tratado por interrupções e rotinas de polling de baixo nível para traduzir o pressionamento de teclas específicas (`W`, `S`, `J`, `K`, `Enter`) em eventos de software consumidos pela máquina de estados do menu.

---

## 2. Subsistema de Armazenamento e Leitura Dinâmica via microSD (FAT32)

Diferente de abordagens tradicionais que gravam páginas web estáticas diretamente na memória flash interna (SPIFFS/LittleFS) — exigindo recompilação de código para qualquer modificação visual —, o EXATA adota um paradigma modular baseado em persistência externa:

* **Camada de Driver VFS (Virtual File System):** O firmware inicializa o barramento SPI/SDMMC para montar cartões de memória formatados em **FAT32** durante a rotina de `setup()`.
* **Roteamento Dinâmico de Diretórios:** O sistema analisa a árvore de diretórios em busca de caminhos estruturados sob a pasta `/sites/`. Cada subdiretório atua como um container de recursos isolados (contendo arquivos do tipo `index.html`, folhas de estilo CSS e scripts legados).
* **Bufferização de I/O:** Os arquivos são lidos em blocos segmentados e injetados diretamente no fluxo de resposta do socket HTTP, permitindo servir conteúdos maiores do que a memória RAM estática disponível no microcontrolador sem ocasionar estouro de pilha (*stack overflow*).

---

## 3. Pilha de Rede, Camada de Enlace e Servidor HTTP Assíncrono

A infraestrutura de comunicação sem fio implementada no firmware transforma o dispositivo em um agente de rede independente:

* **Modo AP (Access Point) Isolado:** O controlador Wi-Fi interno do ESP32-S3 é configurado para operar em modo de infraestrutura sem fio própria, despachando beacons de rede, gerenciando alocações de endereços IP locais via servidor DHCP integrado e estabelecendo a tabela de roteamento interno na sub-rede `192.168.4.1`.
* **Captura de Roteamento e Resolução de Requisições HTTP:** O servidor web interno implementa um interpretador de requisições GET/POST de escopo restrito. Ele processa cabeçalhos HTTP brutos recebidos via sockets TCP, mapeia a URI requisitada para o caminho correspondente no sistema de arquivos do microSD e devolve os códigos de status adequados (`200 OK`, `404 Not Found`).

---

## 4. Pipeline de Integração Contínua e CI/CD (GitHub Actions)

Para mitigar a complexidade de compilação cruzada (*cross-compilation*) de toolchains baseadas em Xtensa/ESP-IDF localmente, o repositório integra fluxos automatizados de engenharia de software:

* **Workflows de Compilação Automatizada:** O diretório `.github/workflows/` abriga arquivos de configuração YAML que acionam runners virtuais isolados a cada alteração enviada ao repositório (*git push* ou criação de tags).
* **Geração de Artefatos Binários:** O ambiente executa o compilador do PlatformIO, baixando dependências de bibliotecas de terceiros de forma determinística, resolvendo flags de otimização de compilador (`-O2`) e gerando o binário monolítico final (`firmware.bin`). Esse arquivo é empacotado e disponibilizado para download direto na aba de liberações (*Releases*), viabilizando o flashing por meio de ferramentas como o *M5Burner* ou utilitários via porta serial.

---

## 5. Matriz de Mapeamento de Controles e Estados Físicos

| Identificador Físico | Função Lógica no Sistema | Tratamento de Interrupção / Rotina |
| :--- | :--- | :--- |
| **`W`** / **`K`** | Decremento do ponteiro de menu | Move o foco seletivo para o índice anterior da matriz de aplicações. |
| **`S`** / **`J`** | Incremento do ponteiro de menu | Move o foco seletivo para o índice posterior da matriz de aplicações. |
| **`Enter`** | Disparo de transição de estado | Confirma a seleção corrente, montando os parâmetros de execução e inicializando os serviços de rede/storage correspondentes. |
