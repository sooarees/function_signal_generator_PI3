# Etapa 1

Nesta etapa definimos a topologia do gerador, seus requisitos iniciais e os principais componentes. Também escolhemos o microcontrolador e a interface, considerando a geração dos sinais, o condicionamento analógico, a alimentação e o controle pelo usuário.

## Desenvolvimento

### 1. Topologia adotada

Antes do diagrama de blocos, avaliamos cristal, multivibrador, DDS e DAC. O cristal é mais indicado para frequência fixa, e o multivibrador tem pouca flexibilidade. O DDS oferece bom controle, mas exige um componente dedicado. Escolhemos o DAC porque permite gerar diferentes formas de onda e fazer os ajustes por software.

Também comparamos o DAC externo com o interno. O externo pode ter maior desempenho, mas adiciona componentes, custo e complexidade. Os professores reforçaram a escolha do DAC interno, que atende aos requisitos iniciais e simplifica o circuito.

A topologia adotada ficou: Um **DAC interno conectado ao PGA (amplificador de ganho programável)** [1][2]. O DAC gera a forma de onda, e o PGA ajusta o ganho antes das etapas de offset e filtragem.

### 2. Estrutura do Circuito
Como a forma de onda será gerada pelo DAC do microcontrolador, não será necessário um circuito analógico oscilador. A parte analógica fica responsável pela amplificação, pelo offset e pela filtragem. O diagrama abaixo mostra a organização desses blocos.

<div align="center">
  <img src="../images/DiagramaDeBlocos_Etapa01.png" alt="Diagrama de blocos" width="650">
</div>

**Descrição:**
*   **STM32G474:** Microcontrolador responsável por gerar os pontos da onda e o sinal de offset via DAC, gerenciar o DMA e controlar a interface.
*   **Display:** TFT touchscreen de 2,4", 320 × 240 pixels e controlador ILI9341 [3]. O LCD utilizará barramento paralelo de 8 bits por GPIO, enquanto o touch resistivo de quatro fios será lido pelos ADCs do microcontrolador.
*   **Amplificador Diferencial:** O INA126 recebe os sinais de dois DACs para gerar a tensão de offset [4].
*   **Buffer:** O TL074 amplifica a tensão e aplica o offset [5].
*   **Filtros:** Suavizam a variação discreta gerada pelo DAC.
*   **HLK-PM12:** Fonte chaveada isolada da Hi-Link para saída de +12V/250mA a partir da rede elétrica [8].
*   **XL6019:** Conversor DC-DC Buck-Boost que gera -12V a partir dos +12V, utilizado na alimentação assimétrica dos AmpOps [7].
*   **LM2596:** Gera os +5V de alimentação do MCU e do display [6].

### 3. Escolha dos Componentes da Interface

Escolhemos um módulo TFT touchscreen de 2,4", com resolução de 320 × 240 pixels, controlador ILI9341 e painel resistivo de quatro fios. Esse módulo já foi usado no projeto [Osciloscópio Portátil](https://github.com/fab-rodrigs/portable-oscilloscope-pi3), que servirá de referência para a integração com a biblioteca LVGL [9]. Pela tela, será possível escolher a forma de onda e ajustar frequência, amplitude e offset.

O ILI9341 aceita mais de uma interface, mas usaremos o modo paralelo de 8 bits do módulo de referência. O LCD usa as linhas D0–D7 e os sinais CS, RS/DC, WR, RD e RST. O touch resistivo não usa SPI: suas quatro linhas alternam entre GPIO e entradas ADC para medir a posição do toque. Como o conjunto ocupa vários pinos, o mapeamento precisa considerar também os DACs, OPAMPs e outros periféricos. O driver do projeto de referência poderá ser aproveitado como base, com as adaptações necessárias do FRDM-K64F para o STM32G474.


### 4. Requisitos e Seleção do Microcontrolador (MCU)

#### 4.1. Requisitos Técnicos
*   Mínimo de 3 canais de DAC internos.
*   Pelo menos um dos DACs com settling time menor do que 5us (para obter pelo menos 10 pontos por período em 20kHz).
*   Controlador DMA para atualização contínua do DAC (mínimo de 200 kSPS).
*   Interface paralela de 8 bits para o LCD.
*   ADC para o touchscreen resistivo.
*   Suporte a modos de baixo consumo de energia.
*   Disponibilidade em placas de desenvolvimento.

#### 4.2. Análise Comparativa

| Critério | ATmega | dsPIC33CK | MSP430FR2355 | ESP32-S3 | RP2350 | **STM32G474** |
|:--|:-:|:-:|:-:|:-:|:-:|:-:|
| **C1** — DAC | Não `0` | Sim `3` | Sim `4` | Não `0` | Não `0` | Sim **`7`** |
| **C2** — Settling time | Não | Sim | Sim | Não | Não | Sim **`3 µs`** |
| **C3** — DMA | Não | Sim | Não | Sim | Sim | Sim |
| **C4** — LCD paralelo 8 bits | Não | Sim | Sim | Sim | Sim | Sim |
| **C5** — ADC | Parcial | Sim | Sim | Parcial | Sim | Sim |
| **C6** — Baixo consumo | Sim | Sim | Sim | Sim | Sim | Sim |
| **C7** — Kit de Desenvolvimento | Sim | Não | Sim | Sim | Sim | Sim |
| **Aprovações** | `2/7` | **`6/7`** | `6/7` | `5/7` | `5/7` | **`7/7`** |

#### 4.3. Conclusão da Seleção
Escolhemos o **STM32G474** porque ele foi o único da comparação que atendeu a todos os requisitos. Ele reúne os DACs, o DMA e os GPIOs necessários, sem exigir componentes extras para a geração do sinal.

### 5. Parâmetros ajustáveis

Para os primeiros testes, definimos quatro formas de onda: senoidal, triangular, quadrada e dente de serra. A amplitude poderá chegar a **10 Vpp**, o offset irá de **−5 V a +5 V** e a frequência máxima será de **20 kHz**, com pelo menos **10 pontos por período** nessa frequência. Esses limites foram adotados como ponto de partida e poderão aumentar de acordo com os resultados dos próximos testes.

## Testes

Nesta etapa, a validação foi feita pela comparação dos componentes e pela consulta aos datasheets. Os testes práticos de geração, amplitude, offset e frequência serão realizados nas próximas etapas, após a montagem do protótipo.

## Referências (links/datasheets/livros)

1. [STM32G474 — datasheet DS12288 (STMicroelectronics)](https://www.st.com/resource/en/datasheet/dm00431551.pdf)
2. [STM32G4 — manual RM0440 (STMicroelectronics)](https://www.st.com/resource/en/reference_manual/dm00355726.pdf)
3. [ILI9341 — datasheet (Ilitek)](https://cdn-shop.adafruit.com/datasheets/ILI9341.pdf)
4. [INA126 — datasheet (Texas Instruments)](https://www.ti.com/lit/ds/symlink/ina126.pdf)
5. [TL074 — datasheet (Texas Instruments)](https://www.ti.com/lit/ds/symlink/tl074.pdf)
6. [LM2596 — datasheet (Texas Instruments)](https://www.ti.com/lit/ds/symlink/lm2596.pdf)
7. [XL6019 — datasheet (XLSEMI)](https://www.xlsemi.com/datasheet/XL6019-CN.pdf)
8. [HLK-PM12 — página do fabricante (Hi-Link)](https://hlktech.net/index.php?id=111)
9. [LVGL — documentação oficial](https://docs.lvgl.io/)
