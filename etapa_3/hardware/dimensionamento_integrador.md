# Dimensionamento do integrador — Etapa 3

O grupo dimensionou a rede de resistores para ampliar a faixa de operação do integrador usado na geração das ondas triangular e dente de serra. O cálculo abaixo usa um modelo ideal e os limites adotados no projeto.

## Modelo do integrador

O integrador ativo com TL084 usa uma referência de $1{,}65\,\text{V}$ na entrada não inversora. A onda quadrada de entrada tem ciclo de trabalho de 50%. No modelo, ela alterna entre $V_{\mathrm{ref}}-A$ e $V_{\mathrm{ref}}+A$, em que $A$ é a amplitude de pico em relação à referência. Assim, a tensão sobre o resistor de entrada alterna entre $-A$ e $+A$. A inclinação da saída tem módulo $A/(RC)$. O modelo segue o circuito integrador descrito por Sedra e Smith [1] e usa o TL084 [2].

Durante meio período, a saída percorre a tensão pico a pico $V_{\mathrm{pp}}$. Para frequência $f$, a inclinação em volts por segundo é dada por

$$
\left|\frac{dV_o}{dt}\right|
= \frac{V_{\mathrm{pp}}}{1/(2f)}
= 2fV_{\mathrm{pp}}
= \frac{A}{RC}.
$$

## Limites de uma malha RC

O dimensionamento adota uma amplitude de pico de entrada entre $10\,\text{mV}$ e $1{,}5\,\text{V}$. Para um mesmo $RC$, a razão entre as inclinações máxima e mínima é $1{,}5/0{,}010=150$. A amplitude pico a pico pretendida na saída varia de $0{,}1$ a $10\,\text{V}$.

Para cobrir toda a faixa de amplitude dentro de uma subfaixa de frequência, o integrador precisa gerar $0{,}1\,\text{V}_{\mathrm{pp}}$ na menor frequência e $10\,\text{V}_{\mathrm{pp}}$ na maior. Esses pontos exigem, respectivamente, a menor e a maior inclinação. A razão máxima entre os limites da subfaixa é

$$
\frac{f_{\max}}{f_{\min}}
= \frac{0{,}1}{10}\times150
= 1{,}5.
$$

Por exemplo, entre 1 Hz e 1,5 Hz, os extremos exigem inclinações de $0{,}2$ e $30\,\text{V/s}$. No modelo ideal, ambos correspondem a $RC=50\,\text{ms}$, usando $10\,\text{mV}$ e $1{,}5\,\text{V}$ de amplitude de pico na entrada, respectivamente.

## Número de subfaixas

Com subfaixas adjacentes de razão 1,5, o limite superior depois de $n$ subfaixas é $1\,\text{Hz}\times1{,}5^n$. Para alcançar 20 kHz, o número mínimo teórico é

$$
n = \left\lceil \frac{\ln(20\,000/1)}{\ln(1{,}5)} \right\rceil
= \left\lceil 24{,}43 \right\rceil
= 25.
$$

## Seleção no esquemático

O projeto prevê quatro multiplexadores CD4051B de oito canais [3] para selecionar 32 combinações resistivas distribuídas entre dois integradores. Um usa um capacitor maior para as frequências mais baixas e o outro, um capacitor menor para as mais altas. Ao distribuir a faixa de 1 Hz a 20 kHz em 32 subfaixas com o mesmo passo geométrico, a razão entre limites sucessivos é

$$
q = \left(\frac{20\,000}{1}\right)^{1/32} \approx 1{,}36.
$$

Como $q$ é menor que 1,5, o modelo prevê sobreposição entre as faixas de operação. O resultado orienta a escolha dos resistores. A validação em bancada das 32 combinações ainda está pendente. Os ensaios registrados da triangular e do dente de serra constam no [README de hardware](./README.md#testes).

## Referências

1. SEDRA, Adel S.; SMITH, Kenneth C. *Microeletrônica*. 7. ed. São Paulo: Pearson Education do Brasil, 2017.
2. [Texas Instruments — TL084 JFET-Input Operational Amplifier](https://www.ti.com/lit/ds/symlink/tl084.pdf)
3. [Texas Instruments — CD4051B CMOS Single 8-Channel Analog Multiplexer/Demultiplexer](https://www.ti.com/lit/ds/symlink/cd4051b.pdf)
