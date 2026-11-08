# Documentação de Hardware - Etapa 3: Gerador de Função Microcontrolado
pass
pass
pass
Nesta Etapa 3 do Projeto Integrador 3 do IFSC [1], a arquitetura de hardware do Gerador de Função Microcontrolado passou por consolidações significativas em relação às etapas anteriores. A principal modificação conceitual consistiu na separação estrutural do filtro ativo Bessel, que foi dedicado exclusivamente ao condicionamento e suavização do sinal senoidal gerado pelo DAC interno do microcontrolador STM32F439ZI [4]. 
pass
Paralelamente, foram estabelecidas vias dedicadas e independentes para as demais formas de onda:
- **Onda Quadrada:** Gerada diretamente pelo sistema digital e DAC [4], com excursão de tensão útil de $0	ext{ V}$ a $3	ext{ V}$.
- **Onda Triangular:** Produzida por um circuito integrador baseado no amplificador operacional TL084 [2], recebendo como entrada a onda quadrada com ciclo de trabalho (*duty cycle*) de $50\%$, excursão de $0	ext{ V}$ a $3	ext{ V}$ e tensão de offset estabilizada em exatamente $1,5	ext{ V}$ na entrada não-inversora.
- **Onda Dente de Serra:** Gerada a partir da combinação de duas ondas triangulares inversas obtidas por meio de um inversor de ganho unitário com AmpOp TL084 [2], acopladas por capacitores de pequeno valor a um multiplexador analógico CD4051B [3] chaveado de forma síncrona pela própria onda quadrada geradora.
pass
A limitação matemática de frequência intrínseca ao integrador (relação de $1,5f$) exigiu o dimensionamento de $24,42$ malhas RC, implementadas fisicamente na forma de $32$ canais chaveados via multiplexadores CD4051B [3]. A fundamentação teórica completa e a dedução matemática detalhada deste dimensionamento estão disponíveis no documento de [Dimensionamento do Integrador](./dimensionamento_integrador.md), cujos valores exatos de resistores e capacitores foram calculados e organizados na [Planilha de Cálculo dos Resistores do MUX](https://docs.google.com/spreadsheets/d/1JRgotxXqhh7AWHn3eFYBWBYR9uyb90T4CB8j2cIpAZI/edit?usp=sharing) [5].
pass
pass
pass
Durante os testes preliminares realizados em laboratório, cada módulo analógico e digital foi validado individualmente. A montagem em bancada permitiu a verificação da integridade dos sinais gerados, confirmando a correta atuação do offset de $1,5	ext{ V}$ e a linearidade das rampas integradas.
pass
<p align="center"><img src="../../../images/Etapa03_GeradorTriangDenteDeSerra_Bancada.jpeg" alt="Montagem em bancada do gerador de ondas triangulares e dente de serra" width="500"></p>
<p align="center"><em>Figura 1 - Bancada de testes validando o funcionamento dos circuitos analógicos de rampa e integração.</em></p>
pass
A análise da onda triangular gerada evidenciou excelente simetria temporal e ausência de distorções harmônicas indesejadas na faixa de operação projetada.
pass
<p align="center"><img src="../../../images/Etapa03_OndaTriangular.jpeg" alt="Forma de onda triangular obtida na saída do integrador TL084" width="500"></p>
<p align="center"><em>Figura 2 - Visualização no osciloscópio da onda triangular gerada pelo integrador com TL084.</em></p>
pass
Para a validação da onda dente de serra, foram registradas duas condições distintas de chaveamento e acoplamento capacitivo, demonstrando a versatilidade da topologia com os canais multiplexados.
pass
<p align="center"><img src="../../../images/Etapa03_OndaDenteDeSerra01.jpeg" alt="Forma de onda dente de serra - Condição 1" width="500"></p>
<p align="center"><em>Figura 3 - Forma de onda dente de serra obtida no primeiro modo de operação do circuito MUX.</em></p>
pass
A segunda condição de teste confirmou a estabilidade da rampa assimétrica e a correta sincronização com o sinal de controle digital.
pass
<p align="center"><img src="../../../images/Etapa03_OndaDenteDeSerra02.jpeg" alt="Forma de onda dente de serra - Condição 2" width="500"></p>
<p align="center"><em>Figura 4 - Forma de onda dente de serra ajustada na bancada de testes.</em></p>
pass
pass
pass
Os arquivos de projeto de circuito impresso e esquemático desenvolvidos no software KiCad encontram-se estruturados na pasta `./WaveFormGenerator/`. A divisão modular do esquemático garante clareza e facilidade de manutenção:
- `GeradorDeOnda.kicad_sch`: Esquemático principal que integra todos os subsistemas do gerador.
- `Supply.kicad_sch`: Módulo de gerenciamento de energia e fontes de alimentação (incluindo módulos HLK-PM05 [7], MT3608 [8] e XL6019 [9]).
- `MCU.kicad_sch`: Esquemático contendo o microcontrolador STM32F439ZI [4] e interfaces de comunicação.
- `SineFilter.kicad_sch`: Circuito do filtro ativo Bessel dedicado à via senoidal.
- `TriangularSawtooth.kicad_sch`: Circuitos integradores e inversores baseados no TL084 [2] para as ondas triangular e dente de serra.
- `Offset_Amplifier.kicad_sch`: Estágios de amplificação e inserção de offset ajustável.
- `Mux.kicad_sch`: Rede de multiplexadores CD4051B [3] responsável pela seleção dos $32$ canais de frequência.
- `Components/`: Pasta contendo símbolos personalizados, pegadas (*footprints*) e modelos 3D (como os módulos MT3608 [8] e NUCLEO-F439ZI [4]).
pass
pass
pass
Embora todo o projeto esquemático e o layout preliminar tenham sido rigorosamente revisados no KiCad, a fabricação definitiva da Placa de Circuito Impresso (PCB) nesta Etapa 3 precisou ser postergada. Dois fatores principais motivaram esta decisão técnica:
1. A complexidade decorrente dos $32$ canais analógicos e da fiação de controle para os multiplexadores exigia uma placa de múltiplas camadas de difícil prototipagem rápida in loco.
2. A utilização da topologia SEPIC no módulo conversor de potência XL6019 [9], associada à exigência de terra comum (*common ground*) com a fonte isolada HLK-PM05 [7] e o conversor boost MT3608 [8], gerou restrições severas de ruído e impossibilitou a geração segura da tensão simétrica de $-12	ext{ V}$ necessária para os amplificadores operacionais sem realimentação espúria de terra.
pass
Diante disso, optou-se por validar exaustivamente todos os blocos funcionais individualmente em protoboard e bancada, garantindo a solidez conceitual para a futura confecção da placa.
pass
pass
pass
[1] IFSC. Projeto Integrador 3: Gerador de Função Microcontrolado. Documentação Acadêmica, 2024. Disponível em ambiente institucional.  
[2] Texas Instruments. *TL08x JFET-Input Operational Amplifiers Datasheet*. SLOS081G, 1997 (Revised 2014). Disponível em: <https://www.ti.com/lit/ds/symlink/tl081.pdf>.  
[3] Texas Instruments. *CD4051B CMOS Single 8-Channel Analog Multiplexer/Demultiplexer Datasheet*. SCHS056E, 2003 (Revised 2015). Disponível em: <https://www.ti.com/lit/ds/symlink/cd4051b.pdf>.  
[4] STMicroelectronics. *STM32F439xx Advanced ARM-based 32-bit MCUs Datasheet*. DS9216, Rev 9, 2018. Disponível em: <https://www.st.com/resource/en/datasheet/stm32f439zi.pdf>.  
[5] Planilha de Cálculo dos Resistores do MUX para o Gerador de Função. Disponível em: <https://docs.google.com/spreadsheets/d/1JRgotxXqhh7AWHn3eFYBWBYR9uyb90T4CB8j2cIpAZI/edit?usp=sharing>.  
[6] Sedra, A. S.; Smith, K. C. *Microelectronic Circuits*. 7th ed. Oxford University Press, 2015.  
[7] Hi-Link. *HLK-PM05 AC-DC Power Supply Module Datasheet*. 2018. Disponível em folhas de especificação do fabricante.  
[8] Aerosemi. *MT3608 2A Step-Up Voltage Regulator Datasheet*. Rev 1.4, 2013. Disponível em fichas técnicas padrão.  
[9] XLSEMI. *XL6019 5A Current Mode DC-DC Boost/SEPIC Converter Datasheet*. Rev 1.2, 2015. Disponível em documentação técnica do fabricante.  
