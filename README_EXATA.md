# EXATA — firmware de estudos para M5Stack Cardputer ADV

## Plataforma
- PlatformIO, ambiente `m5stack-cardputer`
- Arduino / ESP32-S3
- Bibliotecas declaradas em `platformio.ini`: M5Cardputer, M5Unified e M5GFX

## Compilar
1. Abra esta pasta no VS Code com PlatformIO.
2. Execute `pio run -e m5stack-cardputer`.
3. O binário, se a compilação terminar sem erros, será criado em `.pio/build/m5stack-cardputer/firmware.bin`.

## Controles
O Cardputer não possui um conjunto separado de setas físicas. Para evitar depender de `Fn`, esta versão aceita **W/A/S/D** como direções nas telas de menu e listas; `A/D` também navegam horizontalmente. `ENTER` confirma e `DEL` volta. Dentro de campos de texto, as teclas são usadas para digitar.

## Wi-Fi e segurança
- O ponto de acesso usa o SSID `Cardputer-Estudos` e a senha inicial `EXATA2026`.
- Conecte o celular a essa rede e abra `http://192.168.4.1`.
- O navegador de arquivos é limitado a `/EXATA_SD`; não use uma rede aberta para compartilhar arquivos privados. Altere a senha no código antes de distribuir o firmware.

## Cartão microSD
Na inicialização, o firmware tenta montar o SD usando os pinos SPI configurados no `SDManager.cpp` e cria:

```
/EXATA_SD/MATEMATICA
/EXATA_SD/FISICA
/EXATA_SD/QUIMICA
/EXATA_SD/BIOLOGIA
/EXATA_SD/NOTAS
/EXATA_SD/FORMULAS
/EXATA_SD/CARTOES
/EXATA_SD/MODULOS
/EXATA_SD/IMAGENS
/EXATA_SD/DIAGRAMAS
```

O leitor abre arquivos de texto/código e imagens JPG/PNG/BMP dentro dos limites de memória implementados. O gerenciador web restringe operações à pasta `/EXATA_SD`. Ler um arquivo `.py` não executa Python; PDF/PPT/PPTX são armazenados/transferidos, não renderizados integralmente pelo display.

## Recursos presentes nesta base
- Menus de Matemática, Física, Química, tabela periódica, Biologia/arquivos, anotações e transferência por Wi-Fi.
- Interpretador numérico para operações aritméticas, potência, multiplicação implícita, funções trigonométricas em radianos, logaritmos, exponenciais, raiz, valor absoluto e fatorial.
- Solução numérica de equações com limites de busca; não é um CAS simbólico universal.
- Fórmulas de física para os casos codificados em `Physics.cpp`.
- Dados dos 118 elementos e notas sobre ligações/octeto em `Chemistry.cpp`.
- Leitor de textos com quebra de linha, visualização de imagens por buffer em RAM, renomeação de arquivos e notas no SD.
- Servidor web Wi-Fi para operações de arquivo.

## Limitações que não devem ser confundidas com recursos concluídos
Esta é uma base de firmware funcional em desenvolvimento, não uma implementação integral de todos os itens imaginados na especificação. O código ainda não implementa integralmente: solucionador simbólico universal, todas as 440 fórmulas, todos os sólidos 3D, balanceador químico geral, configuração eletrônica por subníveis para os 118 elementos, todos os modelos celulares/biológicos interativos, editor vetorial completo, visualizador nativo de PDF/PPT, todos os controles de configuração/brilho/som, nem uma ponte BLE para arquivos. Esses itens exigem módulos adicionais e testes próprios.

## Hardware e validação
A montagem do cartão SD depende dos pinos físicos e do adaptador utilizados no seu Cardputer ADV. O código foi preparado com a configuração SPI do projeto recebido. Antes de gravar, compile no seu ambiente PlatformIO e confirme no dispositivo o SD, o teclado, a tela e o Wi-Fi. Não é correto afirmar que houve teste no hardware sem executá-lo fisicamente.
