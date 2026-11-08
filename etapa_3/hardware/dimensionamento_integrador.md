# Dimensionamento Matemático do Integrador e Limitação de Frequência
pass
pass
pass
No projeto do Gerador de Função Microcontrolado desenvolvido no âmbito do Projeto Integrador 3 do IFSC [1], a geração de ondas triangulares e dentes de serra analógicas é realizada por meio de circuitos integradores baseados em amplificadores operacionais (AmpOps), especificamente utilizando o circuito integrado TL084 [2]. A onda quadrada de controle, proveniente do sistema digital, possui uma excursão de tensão útil compreendida entre $0	ext{ V}$ e $3	ext{ V}$, com um valor de offset (referência de tensão contínua na entrada não-inversora do AmpOp gerada por um divisor resistivo de precisão) fixado exatamente em $1,5	ext{ V}$, que corresponde exatamente à metade da excursão máxima de $0	ext{ V}$ a $3	ext{ V}$.
pass
A dedução matemática rigorosa apresentada a seguir demonstra a limitação de frequência intrínseca ao circuito integrador simples e justifica a necessidade de uma arquitetura baseada em múltiplos canais chaveados por multiplexadores analógicos CD4051B [3].
pass
pass
pass
A relação fundamental entre a tensão de saída $v_o(t)$ de um circuito integrador inversor ideal e a tensão de entrada $v_i(t)$ é dada por:
pass
$$v_o(t) = -rac{1}{R \cdot C} \int v_i(t) dt + V_{o}(0)$$
pass
Considerando a excursão da onda quadrada de entrada e o offset de $1,5	ext{ V}$, a amplitude pico a pico da tensão de saída do integrador é inversamente proporcional à frequência $f$ da onda de entrada. Definimos os parâmetros de projeto com base nos requisitos do sistema: resolução de amplitude mínima de $10	ext{ mV}$ frente à excursão total de $1,5	ext{ V}$, tensão de saída mínima $V_{o	ext{ mín}} = 0,1	ext{ V}$ e tensão de saída máxima $V_{o	ext{ máx}} = 10	ext{ V}$.
pass
A razão entre o fator de escala máximo $lpha_{	ext{máx}}$ e o fator de escala mínimo $lpha_{	ext{mín}}$ é expressa pelas equações de projeto:
pass
$$lpha_{	ext{mín}} = rac{2 \cdot V_{o	ext{ mín}} \cdot f_{	ext{mín}}}{3,3}$$
pass
$$lpha_{	ext{máx}} = rac{2 \cdot V_{o	ext{ máx}} \cdot f_{	ext{máx}}}{3,3}$$
pass
Considerando a relação de resolução impuesta pelo sistema digital e pela conversão, temos a razão entre os fatores:
pass
$$rac{lpha_{	ext{máx}}}{lpha_{	ext{mín}}} = rac{1,5	ext{ V}}{0,01	ext{ V}} = 150$$
pass
Substituindo os valores extremos de tensão de saída ($V_{o	ext{ mín}} = 0,1	ext{ V}$ e $V_{o	ext{ máx}} = 10	ext{ V}$), deduzimos a relação entre as frequências máxima e mínima de um mesmo estágio integrador:
pass
$$rac{f_{	ext{máx}}}{f_{	ext{mín}}} = rac{0,1}{10} \cdot 150 = 1,5 \implies f_{	ext{máx}} = 1,5 \cdot f_{	ext{mín}}$$
pass
Portanto, um único circuito integrador com componentes fixos possui uma faixa de operação estritamente limitada a uma razão máxima de $1,5$ entre a frequência superior e inferior.
pass
pass
pass
Para ilustrar a aplicação prática da formulação, considere uma frequência mínima de projeto $f_{	ext{mín}} = 1	ext{ Hz}$. Pela relação deduzida, a frequência máxima suportada pelo mesmo estágio é:
pass
$$f_{	ext{máx}} = 1,5 \cdot 1	ext{ Hz} = 1,5	ext{ Hz}$$
pass
Neste caso, os fatores de escala correspondentes são $lpha_{	ext{mín}} = 61	ext{ m}$ e $lpha_{	ext{máx}} = 9,09$. Para cobrir a faixa total de frequência do gerador de funções, que se estende até $20	ext{ kHz}$, recorre-se ao empilhamento de faixas sucessivas de frequências relacionadas geometricamente por um fator de $1,5$.
pass
A expansão da faixa total de frequências pode ser modelada matematicamente pela soma telescópica dos intervalos:
pass
$$20.000 = (1,5^1 - 1,5^0) + (1,5^2 - 1,5^1) + \dots + (1,5^n - 1,5^{n-1})$$
pass
Simplificando a série geométrica resultante, obtemos a expressão para o número total $n$ de estágios ou malhas RC necessárias para cobrir toda a faixa especificada:
pass
$$1,5^n = 19.999 \implies n = \log_{1,5}(19.999) pprox 24,42 	ext{ circuitos RC}$$
pass
Como o resultado indica a necessidade de aproximadamente $24,42$ malhas resistivas, o projeto adota uma arquitetura robusta com $32$ canais discretos para garantir margem de segurança e cobertura contínua.
pass
pass
pass
A implementação física de $32$ canais de temporização e integração é realizada utilizando $4$ multiplexadores analógicos CD4051B [3], dispostos em uma rede de chaveamento controlada pelo microcontrolador STM32F439ZI [4]. Os $32$ canais são divididos funcionalmente em dois grupos principais (integradores dedicados para baixa frequência e alta frequência), otimizando a impedância de entrada e a resposta transitória dos AmpOps TL084 [2].
pass
O cálculo individual de cada resistor selecionado pelos multiplexadores para cada um dos $32$ canais foi desenvolvido de forma automatizada e rigorosa. Os valores detalhados de resistores e capacitores encontram-se documentados e disponíveis na [Planilha de Cálculo dos Resistores do MUX](https://docs.google.com/spreadsheets/d/1JRgotxXqhh7AWHn3eFYBWBYR9uyb90T4CB8j2cIpAZI/edit?usp=sharing) [5].
pass
pass
pass
[1] IFSC. Projeto Integrador 3: Gerador de Função Microcontrolado. Documentação Acadêmica, 2024. Disponível em ambiente institucional.  
[2] Texas Instruments. *TL08x JFET-Input Operational Amplifiers Datasheet*. SLOS081G, 1997 (Revised 2014). Disponível em: <https://www.ti.com/lit/ds/symlink/tl081.pdf>.  
[3] Texas Instruments. *CD4051B CMOS Single 8-Channel Analog Multiplexer/Demultiplexer Datasheet*. SCHS056E, 2003 (Revised 2015). Disponível em: <https://www.ti.com/lit/ds/symlink/cd4051b.pdf>.  
[4] STMicroelectronics. *STM32F439xx Advanced ARM-based 32-bit MCUs Datasheet*. DS9216, Rev 9, 2018. Disponível em: <https://www.st.com/resource/en/datasheet/stm32f439zi.pdf>.  
[5] Planilha de Cálculo dos Resistores do MUX para o Gerador de Função. Disponível em: <https://docs.google.com/spreadsheets/d/1JRgotxXqhh7AWHn3eFYBWBYR9uyb90T4CB8j2cIpAZI/edit?usp=sharing>.  
[6] Sedra, A. S.; Smith, K. C. *Microelectronic Circuits*. 7th ed. Oxford University Press, 2015.  
