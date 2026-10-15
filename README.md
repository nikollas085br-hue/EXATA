O EXATA é um firmware baseado em PlatformIO/Arduino para o M5Stack Cardputer que transforma o dispositivo em um ponto de acesso Wi-Fi autônomo (Hotspot) com servidor web estático local integrado.
A arquitetura e o funcionamento do projeto envolvem os seguintes componentes e fluxos:
 * Arquitetura de Firmware: Desenvolvido em C++ (cerca de 89% do código) utilizando a plataforma PlatformIO, integrando bibliotecas voltadas para o hardware específico do Cardputer (gerenciamento de teclado, tela e controladores de armazenamento).
 * Sistema de Arquivos e microSD: O firmware lê o conteúdo de um cartão microSD formatado em FAT32. Os sites estáticos são organizados individualmente em subpastas dentro do diretório /sites/ (contendo arquivos como index.html), permitindo atualizar ou trocar o conteúdo exibido sem a necessidade de recompilar ou regravar o firmware no microcontrolador.
 * Rede e Portal Captivo: O dispositivo opera criando um access point (ponto de acesso Wi-Fi local) que simula um portal cativo ([http://192.168.4.1](http://192.168.4.1)). Embora tente interceptar rotas de detecção de sistemas operacionais, ele funciona estritamente como um servidor local para arquivos estáticos, sem fornecer roteamento ou acesso à internet externa.
 * Interface e Controles Físicos: A navegação e a interação local nos menus do dispositivo utilizam o teclado integrado do Cardputer:
   * W / K: Movem para o item anterior.
   * S / J: Movem para o item seguinte.
   * Enter: Confirma a seleção ou ação.
 * Compilação e Deploy (CI/CD): O repositório inclui fluxos de automação no GitHub Actions (.github/workflows/) configurados para compilar o arquivo firmware.bin automaticamente a cada push ou tag, gerando os artefatos prontos para gravação e facilitando atualizações remotas de versão.
 * 
