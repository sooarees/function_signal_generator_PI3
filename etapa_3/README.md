# Etapa 3

Nesta etapa avançamos na implementação física e na consolidação da arquitetura do gerador de funções microcontrolado, integrando o firmware multitarefa na placa de desenvolvimento e desenvolvendo os circuitos analógicos de conformação de sinal. O trabalho compreendeu a integração do sistema operacional de tempo real com os periféricos de conversão, a validação experimental dos circuitos geradores de onda quadrada, triangular e dente de serra em bancada, a revisão do esquemático completo no KiCad e a análise metrológica dos limites operacionais do sistema.

## Desenvolvimento

### 1. Firmware e Integração no Microcontrolador

A arquitetura de software foi consolidada na placa NUCLEO-F439ZI com o microcontrolador STM32F439ZI [7] utilizando o sistema operacional FreeRTOS. A estrutura foi dividida em tarefas concorrentes com prioridades estritas: controle do sinal e sincronização (`SignalControl`), recepção de comandos seriais e telemetria (`SerialControl`), e atualização da interface gráfica com leitura do touchscreen (`Interface`). 

A geração do sinal de onda senoidal e da onda quadrada base utiliza o conversor digital-analógico interno DAC1 (canal 1 no pino PA4) sincronizado pelo temporizador TIM6 em interrupção periódica. A interface gráfica opera em orientação paisagem com o controlador de display, permitindo alterar parâmetros como frequência (1 Hz a 20 kHz), amplitude (até 9,8 Vpp) e ciclo de trabalho. Um canal de conversão analógico-digital (ADC2 no canal 4) foi configurado para executar autotestes internos no nó de saída do DAC. A documentação completa da arquitetura do firmware e das tarefas está disponível em [software/README.md](./software/README.md).

### 2. Geração e Condicionamento Analógico das Formas de Onda

Diante das distorções de atenuação e fase observadas na Etapa 2 ao filtrar ondas com bordas rápidas em filtros de reconstrução, a arquitetura analógica foi dividida em duas vias funcionais: uma malha com filtro passa-baixas Sallen-Key de Bessel para a senoide e uma via analógica dedicada para as formas de onda não senoidais.

A onda quadrada é fornecida pelo DAC, permitindo amplo controle de amplitude e ciclo de trabalho. Para a síntese da onda triangular, empregou-se um circuito integrador ativo baseado no amplificador operacional TL084 [1], que recebe a onda quadrada com ciclo de trabalho fixado em 50% e um nível contínuo de referência de 1,65 V na entrada não-inversora, proporcionando integração simétrica com inclinações idênticas.

A onda dente de serra é produzida a partir de duas ondas triangulares defasadas em 180 graus: a triangular original do integrador e uma versão defasada obtida por um estágio inversor de ganho unitário com TL084 [1]. Ambas são conectadas a um multiplexador analógico CD4051B [2] acoplado em corrente alternada. A própria onda quadrada geradora comuta o canal do multiplexador em sincronia com o sinal, selecionando alternadamente os trechos ascendentes das rampas para compor a dente de serra. 

O protótipo de bancada comprovou a viabilidade prática da topologia, como ilustrado na imagem a seguir:

<p align="center">
  <img src="../images/Etapa03_GeradorTriangDenteDeSerra_Bancada.jpeg" alt="Protótipo do gerador em bancada" width="500">
</p>
<p align="center"><em>Montagem experimental do estágio analógico gerador de sinais não senoidais.</em></p>

### 3. Modelagem Matemática e Esquemático no KiCad

Após a validação prática, a modelagem matemática demonstrou que, ao adotar 10 mV como resolução mínima do DAC e exigir excursão de 100 mV a 10 V na saída, um único circuito RC cobre uma janela de frequências restrita a uma razão de 1,5 vezes ($f$ a $1,5f$). Para contemplar de 1 Hz a 20 kHz, seriam necessárias 24,42 malhas discretas. O esquemático integrou essa demanda através de 4 multiplexadores CD4051B [2], permitindo selecionar 32 combinações resistivas com fator de passo de 1,36 vezes. Os cálculos estão detalhados em [hardware/dimensionamento_integrador.md](./hardware/dimensionamento_integrador.md).

O esquemático integrado no KiCad foi organizado hierarquicamente em blocos funcionais (`Supply`, `MCU`, `SineFilter`, `TriangularSawtooth`, `Offset_Amplifier` e `Mux`), com documentação disponível em [hardware/README.md](./hardware/README.md). Durante a revisão técnica, identificou-se que o módulo comercial XL6019 [6] opera em topologia SEPIC de terra comum, impossibilitando a geração do barramento negativo de -12 V sem curto-circuito na alimentação. Devido a essa não conformidade e à complexidade da malha de 32 resistores, optou-se tecnicamente por adiar a confecção da placa de circuito impresso (PCB) definitiva para a Etapa 4, assegurando correções seguras de projeto.

## 4. Circuito de geração da tensão de offset

O circuito de offset converte duas tensões unipolares fornecidas pelos DACs do microcontrolador (0 a 3,3 V) em uma única tensão bipolar, na faixa de aproximadamente −5 V a +5 V, utilizada para deslocar o nível DC do sinal de saída do gerador.

O circuito é composto por três estágios com amplificadores operacionais TL074, alimentados em ±12 V.

### 4.1. Buffers de entrada (U3 e U4)

Cada saída de DAC passa por um seguidor de tensão de ganho unitário. Esses estágios apresentam alta impedância de entrada e baixa impedância de saída, isolando os DACs da malha resistiva seguinte. Assim, o ganho do circuito não depende da impedância de saída dos conversores e os dois ramos do amplificador diferencial ficam equilibrados.

### 4.2. Amplificador diferencial (U5)

Os resistores R6 e R5 (10 kΩ) e R8 e R7 (15 kΩ) formam um subtrator com ganho diferencial definido pela razão entre eles:

$$
V_{OFFSET} = \frac{R_8}{R_6} \cdot (V_4 - V_3) = 1{,}5 \cdot (V_4 - V_3)
$$

Como cada entrada varia de 0 a 3,3 V, a diferença $V_4 - V_3$ varia de −3,3 V a +3,3 V, resultando em uma tensão de offset entre aproximadamente **−4,95 V e +4,95 V**.

O uso de resistores iguais nos dois ramos ($R_6 = R_5$ e $R_8 = R_7$) garante o cancelamento da componente comum às duas entradas. O capacitor C1 (47 pF), em paralelo com R8, limita a banda do estágio e evita oscilações de alta frequência, sem afetar a resposta em corrente contínua.

### 4.3. Buffer de saída (U2)

Um seguidor de tensão entrega a tensão de offset ao estágio somador, garantindo baixa impedância de saída.

### 4.4. Operação

No firmware, adota-se um DAC para cada polaridade: para offset positivo, ajusta-se $V_4$ e mantém-se $V_3$ em 0 V; para offset negativo, o inverso.

| Componente | Valor | Função |
|---|---|---|
| U3, U4 | TL074 (seguidor) | Buffers de entrada dos DACs |
| R6, R5 | 10 kΩ | Resistores de entrada do subtrator |
| R8, R7 | 15 kΩ | Resistores de realimentação e de referência (ganho 1,5) |
| C1 | 47 pF | Limitação de banda / estabilidade |
| U5 | TL074 | Amplificador diferencial |
| U2 | TL074 (seguidor) | Buffer de saída |

Na simulação apresentada, com $V_3 = 1{,}3\ \text{V}$ e $V_4 = 0\ \text{V}$, a saída esperada é:

$$
V_{OFFSET} = 1{,}5 \cdot (0 - 1{,}3) = -1{,}95\ \text{V}
$$

A tensão de offset é então somada ao sinal no amplificador de saída, deslocando o nível DC da forma de onda gerada conforme o valor definido pelo usuário na interface.

## Testes

Os testes laboratoriais validaram individualmente os blocos funcionais do gerador:

| Subsistema avaliado | Resultado | Observações metrológicas |
|:--|:-:|:--|
| Firmware FreeRTOS e DAC | Aprovado | Execução estável das tasks, autoteste via ADC2 com 305 mVpp a 1 kHz |
| Touchscreen e display | Aprovado | Navegação, ajuste de parâmetros e botões operando sem travamento |
| Circuito integrador triangular | Aprovado | Rampas simétricas e lineares comprovadas em osciloscópio |
| Gerador de onda dente de serra | Aprovado | Comutação síncrona por CD4051B validada em duas amplitudes |
| Filtro de reconstrução senoidal | Aprovado | Atenuação adequada de harmônicas discretas do DAC em bancada |
| Fonte chaveada SEPIC (XL6019) | Reprovado | Terra comum inviabiliza -12 V; requer fonte com isolação galvânica |

Os testes confirmaram a solidez do firmware e a conformação de todas as formas de onda analógicas. A reprogramação da malha resistiva e a readequação da fonte simétrica constituem as diretrizes prioritárias para a entrega da PCB na Etapa 4.

## Referências (links/datasheets/livros)

1. [Texas Instruments — TL084 JFET-Input Operational Amplifier Datasheet](https://www.ti.com/lit/ds/symlink/tl084.pdf)
2. [Texas Instruments — CD4051B CMOS Analog Multiplexer/Demultiplexer Datasheet](https://www.ti.com/lit/ds/symlink/cd4051b.pdf)
3. [Texas Instruments — TL081 JFET-Input Operational Amplifier Datasheet](https://www.ti.com/lit/ds/symlink/tl081.pdf)
4. [Hi-Link — HLK-PM05 AC-DC 5V 3W Power Supply Module](https://hlktech.net/index.php?id=109)
5. [Aerosemi — MT3608 2A Step Up Converter Datasheet](https://www.olimex.com/Products/Breadboarding/BB-PWR-3608/resources/MT3608.pdf)
6. [XLSEMI — XL6019 5A Buck-Boost DC-DC Converter Datasheet](https://www.xlsemi.com/datasheet/XL6019-CN.pdf)
7. [STMicroelectronics — STM32F439ZI Datasheet DS9484](https://www.st.com/resource/en/datasheet/stm32f439zi.pdf)
8. SEDRA, Adel S.; SMITH, Kenneth C. *Microeletrônica*. 7. ed. São Paulo: Pearson Education do Brasil, 2017.
