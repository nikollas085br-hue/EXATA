# 📚 Central de Estudos — M5 Cardputer

Um sistema de estudos desenvolvido para o **M5Stack Cardputer**, criado com o objetivo de transformar o dispositivo em uma pequena central portátil de aprendizado, reunindo ferramentas interativas, cálculos, informações científicas e suporte a conteúdos armazenados no cartão SD.

O projeto está em desenvolvimento contínuo e foi pensado para poder crescer conforme novas funções e materiais forem adicionados.

---

## 🎯 Objetivo

A ideia do projeto é aproveitar o hardware compacto do M5 Cardputer para criar uma plataforma de estudos portátil, funcionando de forma independente e com uma interface simples para navegação pelo teclado e pela tela.

Em vez de ser apenas uma calculadora ou um programa com algumas funções isoladas, o sistema busca reunir diferentes ferramentas educacionais em um único dispositivo.

---

## ⚙️ Recursos atuais

### 🧮 Calculadora

O sistema possui uma calculadora integrada para realizar operações matemáticas.

Entre as operações trabalhadas estão:

* Adição
* Subtração
* Multiplicação
* Divisão
* Potenciação
* Operações matemáticas diversas
* Cálculos utilizando funções implementadas pelo sistema

A calculadora também está sendo expandida para receber novas operações e formas de entrada.

---

### 🧪 Tabela Periódica Interativa

Uma das funções principais do projeto é a **Tabela Periódica interativa**.

O usuário pode navegar pelos elementos e consultar informações diretamente no Cardputer.

A ideia é transformar a tabela em uma ferramenta de consulta rápida, permitindo estudar química sem precisar utilizar outro dispositivo.

Entre as informações trabalhadas estão:

* Símbolo químico
* Nome do elemento
* Número atômico
* Organização dos elementos
* Famílias e grupos
* Informações relacionadas às propriedades dos elementos

A estrutura também permite que novas informações sejam adicionadas futuramente.

---

### ⚛️ Ligações Químicas

O sistema também possui recursos relacionados às **ligações químicas**.

A proposta é apresentar de maneira visual e simplificada conceitos como:

* Ligações iônicas
* Ligações covalentes
* Formação de compostos
* Representações de átomos e moléculas
* Relação entre os elementos durante a formação das substâncias

Esse módulo pode ser ampliado futuramente com mais exemplos, modelos e representações.

---

### 📊 Conteúdos e materiais de estudo

O projeto não depende exclusivamente das informações que já estão programadas no firmware.

O **cartão SD** funciona como uma biblioteca externa de materiais.

Isso permite adicionar novos conteúdos ao dispositivo sem precisar modificar todo o programa.

A biblioteca pode receber, por exemplo:

* Anotações
* Imagens
* Gráficos
* Diagramas
* Modelos
* Materiais de revisão
* Conteúdos de diferentes disciplinas
* Outros arquivos compatíveis com o sistema

Dessa forma, o sistema pode continuar crescendo mesmo depois que o firmware estiver instalado.

---

## 💾 Sistema de armazenamento

O cartão SD foi pensado como uma extensão da memória de conteúdo do projeto.

Uma organização possível é:

```text
SD/
├── MATEMATICA/
├── FISICA/
├── QUIMICA/
├── BIOLOGIA/
├── GRAFICOS/
├── MODELOS/
├── DESENHOS/
├── DIAGRAMAS/
├── IMAGENS/
└── MAPAS/
```

A ideia é manter o sistema organizado e permitir que novos materiais sejam adicionados posteriormente.

Isso também evita que todo novo conteúdo precise ser colocado diretamente dentro do código-fonte.

---

## 🖼️ Conteúdo visual

O projeto também foi pensado para trabalhar com conteúdos visuais.

Podem ser utilizados:

* Gráficos matemáticos
* Diagramas científicos
* Modelos de estruturas
* Desenhos educacionais
* Imagens explicativas
* Representações de conceitos
* Esquemas de estudo

Isso permite que o Cardputer seja utilizado não apenas para mostrar textos, mas também como uma ferramenta visual de consulta.

---

## 📚 Possibilidade de expansão

Uma das principais características do projeto é que ele foi desenvolvido pensando em expansão.

Novos módulos podem ser adicionados posteriormente, como:

* Mais ferramentas matemáticas
* Física
* Química
* Biologia
* Ciências da Terra
* Conversores de unidades
* Fórmulas
* Gráficos
* Dicionários científicos
* Sistemas de consulta
* Novas ferramentas de cálculo
* Novos modelos e diagramas

A ideia é que o sistema não fique limitado às funções existentes atualmente.

---

## 🧩 Estrutura do projeto

O código está dividido em diferentes arquivos para facilitar a manutenção e a expansão.

Exemplo da estrutura atual:

```text
src/
├── main.cpp
├── App.cpp
├── Chemistry.cpp
├── MathEngine.cpp
├── Physics.cpp
├── SDManager.cpp
└── WifiManager.cpp
```

Cada módulo possui uma função específica dentro do sistema.

### `main.cpp`

Responsável pela inicialização principal do programa.

### `App.cpp`

Controla a aplicação, menus, navegação e integração das diferentes funções.

### `MathEngine.cpp`

Concentra recursos relacionados à matemática e aos cálculos.

### `Physics.cpp`

Reúne funções e ferramentas relacionadas à física.

### `Chemistry.cpp`

Responsável pelos recursos de química, incluindo ferramentas relacionadas à tabela periódica e ligações químicas.

### `SDManager.cpp`

Gerencia o acesso aos arquivos armazenados no cartão SD.

### `WifiManager.cpp`

Contém a estrutura relacionada às funcionalidades de conectividade Wi-Fi que podem ser utilizadas pelo sistema.

---

## 🖥️ Hardware

O projeto foi desenvolvido para o:

**M5Stack Cardputer**

O dispositivo reúne em um formato compacto:

* Tela
* Teclado físico
* ESP32-S3
* Armazenamento externo por cartão SD
* Conectividade
* Interface portátil

A combinação desses recursos permite criar uma plataforma de estudos pequena e independente.

---

## 🔧 Tecnologias utilizadas

O projeto utiliza principalmente:

* C++
* Arduino Framework
* PlatformIO
* ESP32-S3
* M5Unified
* M5GFX
* M5Cardputer
* SD
* SPI
* Wi-Fi

---

## 🚧 Estado do projeto

O projeto está em **desenvolvimento ativo**.

Algumas funções já estão implementadas e outras ainda estão sendo aprimoradas.

O objetivo não é criar apenas um programa fechado, mas uma plataforma que possa receber novas ferramentas e conteúdos ao longo do tempo.

---

## 💡 Ideia principal

A proposta pode ser resumida em uma ideia simples:

> **Um pequeno computador de estudos que pode crescer junto com os conteúdos adicionados pelo usuário.**

As funções programadas ficam no firmware, enquanto o cartão SD permite ampliar a biblioteca de materiais.

Assim, o projeto combina **programação + hardware + armazenamento externo + ferramentas educacionais** em um único dispositivo.

---

## 🔮 Futuras possibilidades

Algumas ideias para futuras versões incluem:

* Melhorias na interface
* Mais operações matemáticas
* Novos módulos científicos
* Mais informações na tabela periódica
* Novos modelos de ligações químicas
* Visualização de gráficos
* Mais formatos de arquivos
* Sistema de busca de conteúdos
* Organização automática dos materiais
* Novas ferramentas para estudo
* Melhor integração entre o firmware e o cartão SD

---

## 👨‍💻 Desenvolvimento

Este projeto está sendo desenvolvido como um projeto pessoal de programação e experimentação com o **M5Stack Cardputer**.

A ideia é aprender e, ao mesmo tempo, construir uma ferramenta realmente útil para estudos.

O projeto continua recebendo melhorias, correções e novas funcionalidades.

---

## ⭐ Contribuições e sugestões

Sugestões de novas funções, melhorias na interface, ferramentas educacionais e ideias para utilização do hardware são bem-vindas.

Se você tiver uma ideia que poderia tornar essa central de estudos melhor, pode abrir uma **Issue** ou entrar em contato pelo repositório.

---

## 📌 Resumo

**Central de Estudos para M5 Cardputer**

Um sistema portátil que reúne:

* 🧮 Calculadora
* 🧪 Tabela periódica interativa
* ⚛️ Ligações químicas
* 📚 Ferramentas educacionais
* 💾 Biblioteca de conteúdos pelo cartão SD
* 📊 Gráficos e diagramas
* 🖼️ Imagens e modelos
* 🔧 Arquitetura modular
* 🚀 Possibilidade de expansão contínua

O projeto ainda está evoluindo, e a intenção é transformar o Cardputer em uma verdadeira **central portátil de estudos e ferramentas científicas**.
