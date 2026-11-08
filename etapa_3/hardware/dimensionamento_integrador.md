# Dimensionamento Matemático e Limitações do Integrador — Etapa 3

Este documento apresenta a dedução analítica das limitações de largura de faixa para o circuito integrador analógico utilizado na síntese das ondas triangular e dente de serra do gerador de funções, justificando a necessidade de segmentação em múltiplos estágios resistivos.

---

## 1. Contextualização e Equação Base do Integrador

A geração analógica da forma de onda triangular foi concebida a partir da integração temporal de uma onda quadrada periódica com ciclo de trabalho (*duty cycle*) fixo em $50\%$. Essa onda quadrada geradora é produzida pelo conversor digital-analógico (DAC) do microcontrolador com excursão de $0\text{ V}$ a $3,3\text{ V}$. 

Para que a integração ocorra de forma simétrica em torno do zero virtual, aplica-se à entrada não-inversora do amplificador operacional uma tensão contínua de referência igual à metade da amplitude do sinal de entrada ($V_{\text{ref}} = 1,65\text{ V}$). Dessa forma, o sinal visto pela entrada inversora varia simetricamente entre $+1,65\text{ V}$ e $-1,65\text{ V}$.

Durante meio período de integração ($t_{\text{int}} = \frac{T}{2} = \frac{1}{2f}$), a saída do integrador descreve uma rampa linear cuja excursão total pico a pico é designada por $V_o$. Definindo a taxa de variação da rampa como $\alpha$ (em $\text{V/s}$), tem-se:

$$\alpha = \frac{\Delta V_o}{\Delta t} = \frac{V_o}{T / 2} = \frac{2 \cdot V_o}{T} = 2 \cdot V_o \cdot f$$

Considerando que a amplitude da onda quadrada na entrada do integrador é controlada pelo DAC em uma faixa máxima de $3,3\text{ V}$, normaliza-se o parâmetro de ganho do integrador pela excursão de controle:

$$\alpha = \frac{2 \cdot V_o \cdot f}{3,3}$$

Onde:
* $V_o$ é a amplitude pico a pico da onda triangular desejada na saída ($\text{V}$);
* $f$ é a frequência fundamental do sinal ($\text{Hz}$);
* $3,3\text{ V}$ é a tensão de fundo de escala do DAC.

---

## 2. Limites de Operação para um Único Conjunto RC

Os requisitos gerais de projeto estabelecem:
* Faixa total de frequências: $f_{\text{mín}} = 1\text{ Hz}$ até $f_{\text{máx}} = 20\text{ kHz}$;
* Faixa de amplitudes de saída: $V_{o\text{ mín}} = 100\text{ mV}$ ($0,1\text{ V}$) até $V_{o\text{ máx}} = 10\text{ V}$.

Na prática, a amplitude da onda quadrada fornecida pelo DAC não pode assumir valores arbitrariamente pequenos devido à resolução efetiva e ao piso de ruído do conversor. Adotando $10\text{ mV}$ como o menor degrau de amplitude confiável na entrada e $1,5\text{ V}$ como a excursão máxima útil em torno da referência de $1,65\text{ V}$ (garantindo margem linear sem ceifamento), define-se a razão dinâmica máxima de controle de amplitude por software ($\alpha_{\text{máx}} / \alpha_{\text{mín}}$) para uma dada malha como:

$$\frac{\alpha_{\text{máx}}}{\alpha_{\text{mín}}} = \frac{1,5\text{ V}}{10\text{ mV}} = 150$$

Para um único conjunto de componentes passivos $R$ e $C$, a taxa de integração mínima e máxima requeridas nos extremos de amplitude e frequência são dadas por:

$$\alpha_{\text{mín}} = \frac{2 \cdot V_{o\text{ mín}} \cdot f_{\text{mín}}}{3,3}$$

$$\alpha_{\text{máx}} = \frac{2 \cdot V_{o\text{ máx}} \cdot f_{\text{máx}}}{3,3}$$

Dividindo a expressão de $\alpha_{\text{máx}}$ por $\alpha_{\text{mín}}$ para uma mesma configuração física de circuito:

$$\frac{\alpha_{\text{máx}}}{\alpha_{\text{mín}}} = \frac{V_{o\text{ máx}} \cdot f_{\text{máx}}}{V_{o\text{ mín}} \cdot f_{\text{mín}}}$$

Isolando a razão entre as frequências operacionais máxima e mínima permitidas:

$$\frac{f_{\text{máx}}}{f_{\text{mín}}} = \frac{V_{o\text{ mín}}}{V_{o\text{ máx}}} \cdot \frac{\alpha_{\text{máx}}}{\alpha_{\text{mín}}}$$

Substituindo os valores nominais de projeto ($V_{o\text{ mín}} = 0,1\text{ V}$, $V_{o\text{ máx}} = 10\text{ V}$ e razão dinâmica máxima igual a $150$):

$$\frac{f_{\text{máx}}}{f_{\text{mín}}} = \frac{0,1}{10} \cdot 150 = 0,01 \cdot 150 = 1,5$$

$$\implies f_{\text{máx}} = 1,5 \cdot f_{\text{mín}}$$

Esse resultado revela uma restrição crítica: **um único circuito integrador RC fixo só consegue cobrir uma faixa de frequências onde o limite superior é apenas $1,5$ vez o limite inferior ($f$ a $1,5f$)**, caso se deseje manter a capacidade total de ajuste de amplitude de $100\text{ mV}$ a $10\text{ V}$.

### Exemplo Numérico para a Primeira Faixa ($1\text{ Hz}$ a $1,5\text{ Hz}$)

Adotando $f_{\text{mín}} = 1\text{ Hz}$, a frequência máxima alcançável sem troca de componentes é $f_{\text{máx}} = 1,5\text{ Hz}$:

$$\alpha_{\text{mín}} = \frac{2 \cdot 0,1 \cdot 1}{3,3} = \frac{0,2}{3,3} \approx 60,6\text{ mV/s} \quad (61\text{ m})$$

$$\alpha_{\text{máx}} = \frac{2 \cdot 10 \cdot 1,5}{3,3} = \frac{30}{3,3} \approx 9,09\text{ V/s}$$

---

## 3. Cálculo do Número de Malhas RC Necessárias (1 Hz a 20 kHz)

Como cada estágio RC cobre um fator multiplicativo de apenas $1,5\times$ em frequência, cobrir todo o espectro de $1\text{ Hz}$ até $20.000\text{ Hz}$ exige o escalonamento em uma progressão geométrica telescópica de $n$ subfaixas adjacentes:

$$20.000 = (1,5^1 - 1,5^0) + (1,5^2 - 1,5^1) + (1,5^3 - 1,5^2) + \dots + (1,5^n - 1,5^{n-1})$$

A simplificação da série telescópica resulta em:

$$20.000 = 1,5^n - 1,5^0$$

$$1,5^n = 20.000 - 1 = 19.999$$

Aplicando logaritmo em ambos os lados:

$$n = \log_{1,5}(19.999) = \frac{\ln(19.999)}{\ln(1,5)} \approx \frac{9,903437}{0,405465} \approx 24,42\text{ circuitos RC}$$

Portanto, seriam necessárias teoricamente pelo menos **25 combinações RC discretas** para cobrir continuamente de $1\text{ Hz}$ a $20\text{ kHz}$ atendendo simultaneamente a todas as especificações de amplitude e tensão de entrada.

---

## 4. Adaptação no Esquemático para 32 Canais

Para viabilizar a seleção dessas malhas com componentes comerciais, optou-se pela utilização de multiplexadores analógicos de 8 canais `CD4051B`. Quatro circuitos integrados em paralelo fornecem $4 \times 8 = 32$ canais de seleção resistiva:

$$x^{32} = 19.999 \implies x = \sqrt[32]{19.999} \approx 1,36$$

Com 32 resistores selecionáveis divididos entre dois integradores (um com capacitor de maior capacitância para baixas frequências e outro de menor capacitância para altas frequências), cada subfaixa opera com uma razão de passo de $1,36\times$, garantindo sobreposição segura entre as faixas adjacentes ao longo de todo o espectro.

---

## Referências

1. SEDRA, Adel S.; SMITH, Kenneth C. *Microeletrônica*. 7. ed. São Paulo: Pearson Education do Brasil, 2017.
2. [Texas Instruments — CD4051B CMOS Single 8-Channel Analog Multiplexer/Demultiplexer Datasheet](https://www.ti.com/lit/ds/symlink/cd4051b.pdf)
3. [Texas Instruments — TL084 JFET-Input Operational Amplifier Datasheet](https://www.ti.com/lit/ds/symlink/tl084.pdf)