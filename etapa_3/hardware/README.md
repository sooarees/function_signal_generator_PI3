# Hardware da Etapa 3

O desenvolvimento do hardware concentrou-se nos blocos analógicos que recebem os sinais gerados pelo STM32F439ZI [4] e realizam a conversão, o condicionamento e a seleção da forma de onda. A arquitetura separa o caminho senoidal do caminho das formas não senoidais, pois os testes da Etapa 2 mostraram que um único filtro de reconstrução não preservava adequadamente sinais com transições rápidas.

Nesta etapa, os circuitos foram simulados, montados e avaliados separadamente em bancada. Também foi elaborado um esquemático hierárquico no KiCad e produzido um protótipo impresso do gabinete. A cadeia completa entre firmware, condicionamento analógico e saída final ainda não foi validada como um único sistema.

## Desenvolvimento

### Arquitetura de geração das formas de onda

A senoide é produzida diretamente pelo DAC do microcontrolador e possui um caminho próprio de filtragem. A onda quadrada também sai do DAC e pode ser entregue diretamente ao estágio de seleção. Para gerar a triangular e a dente de serra, o firmware fornece uma onda quadrada com duty cycle fixo em 50%, que é transformada por circuitos analógicos.

O integrador ativo utiliza amplificadores operacionais TL084 [1]. A entrada não inversora recebe uma referência de 1,65 V, correspondente ao centro da excursão de 0 a 3,3 V do DAC. Assim, a diferença aplicada ao integrador alterna de forma aproximadamente simétrica e produz rampas ascendentes e descendentes.

A dente de serra é obtida a partir de duas triangulares defasadas em 180 graus. A primeira vem diretamente do integrador e a segunda passa por um inversor de ganho unitário. Um multiplexador analógico CD4051B [2], comutado em sincronismo com a onda quadrada, seleciona alternadamente trechos das duas rampas.

O protótipo utilizado para validar essa topologia foi alimentado por um gerador de funções de laboratório. Portanto, os registros a seguir demonstram o funcionamento do circuito analógico, mas não comprovam uma integração direta com a onda quadrada produzida pelo STM32. A montagem utilizada é apresentada na Figura 1.

<p align="center">
  <img src="../../images/Etapa03_GeradorTriangDenteDeSerra_Bancada.jpeg" alt="Montagem em bancada do gerador triangular e dente de serra" width="400">
</p>
<p align="center"><em>Figura 1 — Montagem experimental do integrador, inversor e multiplexador usados na geração das formas não senoidais.</em></p>

A saída do integrador apresentou rampas lineares e simétricas no ensaio registrado na Figura 2.

<p align="center">
  <img src="../../images/Etapa03_OndaTriangular.jpeg" alt="Onda triangular medida no osciloscópio" width="400">
</p>
<p align="center"><em>Figura 2 — Onda triangular medida na saída do integrador, com rampas aproximadamente lineares e simétricas.</em></p>

As Figuras 3 e 4 registram a saída do multiplexador em dois níveis de amplitude. A forma dente de serra foi mantida enquanto a amplitude do sinal de entrada foi alterada.

<p align="center">
  <img src="../../images/Etapa03_OndaDenteDeSerra01.jpeg" alt="Onda dente de serra no primeiro nível de amplitude" width="400">
</p>
<p align="center"><em>Figura 3 — Onda dente de serra obtida no primeiro nível de amplitude avaliado.</em></p>

<p align="center">
  <img src="../../images/Etapa03_OndaDenteDeSerra02.jpeg" alt="Onda dente de serra no segundo nível de amplitude" width="400">
</p>
<p align="center"><em>Figura 4 — Onda dente de serra obtida após o aumento da amplitude de entrada.</em></p>

### Dimensionamento do integrador

Para uma onda quadrada com duty cycle de 50%, a saída do integrador percorre a excursão pico a pico durante metade do período. A taxa de variação necessária pode ser relacionada à amplitude desejada e à frequência conforme o modelo de integrador ativo [5]. Ao considerar uma resolução prática mínima de 10 mV para o controle e uma excursão útil de 1,5 V em torno da referência, obteve-se uma razão dinâmica de 150 para uma mesma malha RC.

Com a faixa de saída estipulada entre 0,1 V e 10 V, essa razão permite que um conjunto RC cubra apenas uma faixa de frequência cujo limite superior é aproximadamente 1,5 vezes o limite inferior. A cobertura contínua de 1 Hz a 20 kHz exigiria, pelo modelo adotado, pelo menos 25 combinações discretas. O esquemático prevê quatro CD4051B, totalizando 32 canais resistivos e uma razão aproximada de 1,36 entre faixas adjacentes.

A dedução completa, as hipóteses e os cálculos estão documentados no [memorial de dimensionamento do integrador](./dimensionamento_integrador.md). Esses valores representam o dimensionamento teórico; não houve validação experimental das 32 combinações em toda a faixa de frequência e amplitude.

### Estágio de offset e amplificação

O estágio final foi projetado com amplificadores TL084 e TL081 [1][3] para amplificar o sinal e somar uma tensão contínua de offset. O circuito é dividido em buffers de entrada, amplificador diferencial para geração do offset, buffer de saída e somador não inversor. A simulação utilizada no desenvolvimento é apresentada na Figura 5.

<p align="center">
  <img src="../../images/Etapa03_OffsetAmplificador_LTspice.png" alt="Circuito de offset e amplificação simulado no LTspice" width="550">
</p>
<p align="center"><em>Figura 5 — Circuito de geração de offset e amplificação de saída simulado no LTspice.</em></p>

No bloco de offset, duas tensões unipolares passam por seguidores de tensão e são aplicadas a um amplificador diferencial. Com resistores de 10 kΩ nas entradas e 15 kΩ na realimentação e na referência, o ganho diferencial teórico é 1,5. Dessa forma, uma diferença de −3,3 V a +3,3 V entre as entradas corresponde a um offset teórico de aproximadamente −4,95 V a +4,95 V.

O somador não inversor utiliza resistores de 10 kΩ e 36 kΩ. A relação teórica resulta em ganho de 3,6 para o sinal e ganho unitário para o offset. Esse valor deve ser distinguido do ganho conservador de 3,5 usado no firmware e do ganho aproximado de 3,15 medido na montagem em protoboard.

O projeto inclui um capacitor de acoplamento de 82 nF em série com a entrada do sinal. Com resistência equivalente de 46 kΩ, a frequência de corte calculada é próxima de 42 Hz. Como esse valor é superior ao limite inferior pretendido de 1 Hz, a combinação não atende toda a faixa especificada e precisa ser revista antes da integração final. O capacitor bloqueia a componente contínua de aproximadamente 1,65 V presente na saída do DAC, mas também atenua sinais de baixa frequência.

O circuito foi montado em protoboard e alimentado em ±12 V, conforme mostra a Figura 6. O ensaio utilizou uma senoide de aproximadamente 2,08 Vpp e 4,5 kHz proveniente de um gerador de funções de laboratório.

<p align="center">
  <img src="../../images/Etapa03_OffsetAmplificador_Bancada.jpeg" alt="Montagem do circuito de offset e amplificação em protoboard" width="400">
</p>
<p align="center"><em>Figura 6 — Montagem em protoboard dos estágios de offset e amplificação.</em></p>

As Figuras 7, 8 e 9 mostram deslocamentos negativo, nulo e positivo. A amplitude de saída permaneceu próxima de 6,56 Vpp, enquanto o nível contínuo foi modificado.

<p align="center">
  <img src="../../images/Etapa03_OffsetAmplificador_OffsetNegativo.jpeg" alt="Saída amplificada com offset negativo" width="400">
</p>
<p align="center"><em>Figura 7 — Saída amplificada com offset negativo.</em></p>

<p align="center">
  <img src="../../images/Etapa03_OffsetAmplificador_OffsetNulo.jpeg" alt="Saída amplificada com offset nulo" width="400">
</p>
<p align="center"><em>Figura 8 — Saída amplificada com offset nulo.</em></p>

<p align="center">
  <img src="../../images/Etapa03_OffsetAmplificador_OffsetPositivo.jpeg" alt="Saída amplificada com offset positivo" width="400">
</p>
<p align="center"><em>Figura 9 — Saída amplificada com offset positivo.</em></p>

O ganho experimental foi calculado pela razão entre 6,56 Vpp na saída e 2,08 Vpp na entrada, resultando em aproximadamente 3,15. A diferença de cerca de 12% em relação ao ganho teórico de 3,6 pode estar associada às tolerâncias dos resistores e às incertezas da medição. Os valores de offset foram estimados pela posição das formas de onda em relação à referência de zero do osciloscópio.

O firmware da Etapa 3 apenas valida e armazena o valor de offset selecionado na interface. Os ensaios deste circuito utilizaram fontes externas e não representam atuação elétrica do offset comandada pelo STM32.

### Esquemático no KiCad

O projeto foi organizado em folhas hierárquicas no KiCad. A folha principal está disponível em [`GeradorDeOnda.kicad_sch`](./WaveFormGenerator/GeradorDeOnda.kicad_sch), e o projeto completo pode ser aberto por [`GeradorDeOnda.kicad_pro`](./WaveFormGenerator/GeradorDeOnda.kicad_pro). As folhas estão divididas por função:

| Folha | Conteúdo principal |
|:--|:--|
| `MCU.kicad_sch` | Conexões da NUCLEO-F439ZI |
| `SineFilter.kicad_sch` | Filtro ativo do caminho senoidal |
| `TriangularSawtooth.kicad_sch` | Integradores, inversores e seleção resistiva |
| `Offset_Amplifier.kicad_sch` | Geração do offset e amplificação |
| `Mux.kicad_sch` | Seleção analógica da forma de onda |

O esquemático consolida as conexões e os componentes estudados, mas não deve ser interpretado como uma placa definitiva fabricada. A fabricação da PCB foi adiada e os blocos foram avaliados separadamente em protoboard.

### Gabinete

O gabinete foi modelado para acomodar o microcontrolador, o display, os circuitos analógicos e os conectores. O painel frontal removível, apresentado na Figura 10, possui abertura chanfrada para o display e seis pontos de fixação, distribuídos para resistir à pressão aplicada durante o uso do touchscreen.

<p align="center">
  <img src="../../images/Etapa03_Gabinete_PainelFrontal.png" alt="Painel frontal do gabinete" width="360">
</p>
<p align="center"><em>Figura 10 — Painel frontal com abertura para o display e furos de fixação.</em></p>

As laterais das Figuras 11 e 12 foram reservadas para a entrada de alimentação e para o conector BNC de saída, mantendo os caminhos fisicamente separados.

<p align="center">
  <img src="../../images/Etapa03_Gabinete_LateralEsquerda.png" alt="Lateral esquerda do gabinete" width="300">
</p>
<p align="center"><em>Figura 11 — Lateral esquerda com abertura para o conector de alimentação.</em></p>

<p align="center">
  <img src="../../images/Etapa03_Gabinete_LateralDireita.png" alt="Lateral direita do gabinete" width="360">
</p>
<p align="center"><em>Figura 12 — Lateral direita com abertura para o conector BNC de saída.</em></p>

O corpo e o painel frontal foram impressos em peças separadas, como mostra a Figura 13. A impressão permitiu verificar dimensões, encaixe, alinhamento dos furos e abertura do display antes da montagem dos componentes.

<p align="center">
  <img src="../../images/Etapa03_Gabinete_Impresso.png" alt="Corpo e painel frontal impressos em 3D" width="380">
</p>
<p align="center"><em>Figura 13 — Protótipo do gabinete impresso em 3D.</em></p>

## Testes

| Subsistema avaliado | Resultado | Escopo da validação |
|:--|:-:|:--|
| Integrador triangular | Aprovado em bancada | Linearidade e simetria visual das rampas |
| Gerador de dente de serra | Aprovado em bancada | Comutação pelo CD4051B em duas amplitudes |
| Offset | Aprovado separadamente | Deslocamentos negativo, nulo e positivo |
| Amplificador de saída | Aprovado separadamente | 2,08 Vpp na entrada e 6,56 Vpp na saída |
| Seleção de 32 faixas RC | Não validada integralmente | Dimensionamento teórico e esquemático |
| Filtro senoidal | Sem evidência conclusiva nesta etapa | Não classificado como aprovado |
| Integração com o STM32 | Parcial | Firmware e circuitos analógicos testados separadamente |
| PCB definitiva | Não fabricada | Projeto mantido no KiCad para revisão |
| Gabinete | Protótipo aprovado | Impressão e encaixe mecânico verificados |

Os testes confirmaram a viabilidade dos blocos triangular, dente de serra, offset e amplificação. Entretanto, os resultados não demonstram operação contínua em toda a faixa de 1 Hz a 20 kHz nem a cadeia completa comandada pelo firmware. A análise do filtro senoidal também permanece sem evidência experimental suficiente para classificá-lo como aprovado.

## Referências

1. [Texas Instruments — TL084 JFET-Input Operational Amplifier](https://www.ti.com/lit/ds/symlink/tl084.pdf)
2. [Texas Instruments — CD4051B CMOS Analog Multiplexer/Demultiplexer](https://www.ti.com/lit/ds/symlink/cd4051b.pdf)
3. [Texas Instruments — TL081 JFET-Input Operational Amplifier](https://www.ti.com/lit/ds/symlink/tl081.pdf)
4. [STMicroelectronics — STM32F439ZI](https://www.st.com/resource/en/datasheet/stm32f439zi.pdf)
5. SEDRA, Adel S.; SMITH, Kenneth C. *Microeletrônica*. 7. ed. São Paulo: Pearson Education do Brasil, 2017.
