# EXATA — nova interface para o M5Stack Cardputer ADV

Esta pasta é uma reconstrução da camada de interface da EXATA. A tela inicial usa um carrossel de ferramentas com destaque central, pequenos indicadores laterais, barra superior e rodapé de atalhos. O menu não é a antiga grade de três cartões.

## Estado desta etapa

- `src/App.cpp`: interface e navegação reescritas para esta versão.
- Navegação sem `Fn`: `,`/`/` ou `A`/`D` para esquerda/direita; `W`/`S` para listas; `ENTER` abre; `DEL` volta ou apaga.
- Tela inicial com atalhos numéricos para as primeiras áreas.
- Telas para calculadora, equações, gráfico amostrado, física, tabela periódica/ficha do elemento, biologia, tabelas de consulta, estudo, biblioteca SD, visualização de texto, Wi-Fi e configurações.
- `src/SDManager.cpp`: camada SD reorganizada para criar `/EXATA_SD` e suas pastas de estudo.
- `src/WifiManager.cpp`: interface de arquivos pela rede local limitada a `/EXATA_SD`; ponto de acesso `Cardputer-Estudos`, senha `EXATA2026`.
- Os módulos de cálculo e a base de elementos continuam como motores de domínio herdados da etapa anterior; a interface nova chama esses motores. Eles ainda precisam de uma rodada separada de refatoração e testes.

## Compilar

No ambiente com PlatformIO:

```sh
pio run -e m5stack-cardputer
```

Para gravar, conecte o Cardputer e execute:

```sh
pio run -e m5stack-cardputer -t upload
```

## Limites conhecidos desta etapa

Este pacote **não deve ser tratado como firmware final validado**. O ambiente de trabalho não tem PlatformIO instalado e não foi possível executar a compilação real nem testar o teclado no aparelho. O layout e o fluxo de navegação foram escritos, mas precisam de teste no Cardputer ADV. A seleção de opções e a leitura do cartão dependem da revisão do mapa de teclas e dos pinos SD reais do aparelho.

Também não estão concluídos nesta etapa: resolução simbólica universal, balanceador químico, animações científicas avançadas, relógio persistente, Pomodoro com alarme, Bluetooth para arquivos, integração com SymPy no celular e editor gráfico livre. Esses itens precisam ser implementados e testados antes de serem anunciados como concluídos.
