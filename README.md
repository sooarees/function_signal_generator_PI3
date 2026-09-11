# Gerador de Função Microcontrolado

Esse projeto consiste em um gerador de funções microcontrolado capaz de produzir ondas senoidais, triangulares, quadradas e dente de serra. A forma de onda, a frequência, a amplitude e o offset poderão ser ajustados diretamente pelo display touchscreen. O desenvolvimento foi dividido em quatro etapas:

- [Etapa 1](./etapa_1/README.md) (10/09/2026): Pesquisa e estruturação do projeto, definição de parâmetros e escolha dos principais componentes.
- [Etapa 2](./etapa_2/README.md) (data da entrega): Testes em protoboard e desenvolvimento da interface básica.
- [Etapa 3](./etapa_3/README.md) (data da entrega): Protótipo desenvolvido em PCI com case.
- [Etapa 4](./etapa_4/README.md) (data da entrega): Produto final com interface integrada e case definitivo.

## Requisitos

O projeto prevê a utilização dos seguintes módulos, softwares e componentes:

- Microcontrolador STM32G474.
- DACs e amplificadores operacionais internos do STM32G474.
- Display TFT touchscreen de 2,4", com controlador ILI9341 e resolução de 320 × 240 pixels.
- Interface paralela de 8 bits para o LCD e leitura do touch resistivo por GPIO e ADC.
- Estágio analógico externo para amplificação, offset e filtragem.
- Fontes HLK-PM12, XL6019 e LM2596.
- Biblioteca gráfica LVGL, Visual Studio Code, OnShape e KiCAD.

## Visão geral

O STM32G474 será responsável pela geração dos sinais por meio de seus DACs internos, pelo ajuste dos parâmetros e pelo controle da interface. Os amplificadores e filtros realizarão o condicionamento do sinal, enquanto o display permitirá a interação com o usuário. A alimentação proveniente da rede elétrica será convertida nas tensões necessárias aos blocos digitais e analógicos do circuito. O diagrama de blocos abaixo mostra como esses elementos estão ligados.

<div align="center">
  <img src="./images/DiagramaDeBlocos_Etapa01.png" alt="Diagrama de blocos" width="650">
</div>

## Protótipo

**(Adicionar aqui uma foto do protótipo com UM parágrafo de explicação.)**

**A figura deve estar com um tamanho adequado para melhorar a apresentação da página inicial do projeto.**

