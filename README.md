# HOTSPOT ADV UNIVERSAL

Firmware-fonte PlatformIO/Arduino para M5Stack Cardputer. O aparelho cria uma rede Wi-Fi local e serve um site estático escolhido no cartão microSD. O conteúdo do site pode ser trocado sem recompilar o firmware.

## Gerar o arquivo BIN pelo GitHub

1. Crie um repositório no GitHub e envie **todos os arquivos e pastas deste projeto**, incluindo a pasta oculta `.github/workflows/`.
2. Abra a aba **Actions** do repositório e selecione **Compilar HOTSPOT ADV UNIVERSAL**.
3. Aguarde o trabalho terminar. O resultado precisa aparecer com um visto verde.
4. Abra a execução concluída e, na seção **Artifacts**, baixe `hotspot-adv-universal-firmware`.
5. Extraia o ZIP baixado. Dentro estará `firmware.bin`.

A compilação também é iniciada automaticamente a cada `push`. Para criar uma Release com o BIN anexado, crie e envie uma tag como `v1.0.0`.

> **Importante:** o workflow automatiza a compilação, mas não é uma prova de que o firmware foi testado fisicamente. Se a compilação falhar, abra a execução em Actions e consulte o primeiro erro vermelho. Não grave um BIN de uma execução que falhou.

## Compilar localmente

Instale o VS Code e a extensão PlatformIO, abra a pasta deste projeto e execute:

```bash
pio run -e m5stack-cardputer
```

O arquivo gerado ficará em `.pio/build/m5stack-cardputer/firmware.bin`.

## Cartão microSD

Formate o cartão em FAT32 e copie o conteúdo da pasta `sdcard/` para a raiz do cartão. O exemplo fica em `/sites/exemplo/index.html`. Cada site deve ficar em sua própria pasta dentro de `/sites/` e conter `index.html`.

## Controles

- `W` ou `K`: item anterior.
- `S` ou `J`: item seguinte.
- `Enter`: selecionar.
- Nas telas de entrada, digite o valor e pressione Enter; `Q` cancela.

## Acesso ao site

Depois de iniciar o hotspot, conecte o celular à rede configurada e abra `http://192.168.4.1`. O portal cativo tenta atender algumas rotas de detecção, mas o redirecionamento automático varia conforme o sistema operacional e não pode interceptar páginas HTTPS.

## Limitações e compatibilidade

- Este é um servidor de **sites estáticos locais**, não um proxy para a internet. A rede local não fornece internet externa por si só.
- O contador indica dispositivos associados ao ponto de acesso; não representa necessariamente visitantes que abriram o site.
- Use sites leves; memória, tamanho dos arquivos e desempenho do microSD limitam o conteúdo possível.
- A configuração de pinos SD e a definição da placa precisam corresponder à variante exata do Cardputer. O alvo PlatformIO configurado é `m5stack-cardputer`; se o seu Cardputer ADV exigir outro alvo oficial, ajuste `platformio.ini` antes de compilar.
- Use a rede apenas com autorização e escolha uma senha forte para ambientes compartilhados.
