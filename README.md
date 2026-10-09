# EXATA — M5 Cardputer ADV

Projeto inicial em C++/PlatformIO para compilar firmware do M5Stack Cardputer (ESP32-S3) via GitHub Actions.

## Teclas
- `M`: menu de atalhos / voltar
- `G`: gráfico da função de demonstração
- `H`: ajuda
- `W/A/S/D`: deslocar a janela do gráfico
- `+` / `-`: zoom
- `C`, `T`, `S`, `R`, `P`, `L`, `N`, `E`: atalhos matemáticos exibidos no menu

## Gerar o BIN pelo GitHub
1. Crie um repositório GitHub e envie todos os arquivos desta pasta.
2. Abra **Actions** e selecione **Build EXATA firmware**.
3. Clique em **Run workflow** (ou envie um commit para `main`/`master`).
4. Quando terminar, abra a execução e baixe o artefato `EXATA-M5Cardputer-firmware`.
5. Dentro do ZIP do artefato estará `firmware.bin`.

## Observação importante
O código fornecido originalmente desenha o gráfico de uma função específica. Este pacote organiza a interface e os atalhos visuais, mas ainda **não** implementa um analisador universal capaz de calcular qualquer expressão digitada. Os atalhos nesta versão são referências informativas, não inserem resultados em um editor de expressão. Essa parte exige um motor de expressões separado.
