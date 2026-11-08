# Etapa 3

O grupo validou a interface touchscreen e ensaiou o filtro senoidal com o DAC do STM32 de 1 Hz a 20 kHz. Os demais blocos analógicos foram testados separadamente. O projeto da placa de circuito impresso (PCI) avançou até o esquemático no KiCad. O gabinete foi impresso para testar o material e a posição dos conectores.

## Integração do firmware de interface com o circuito analógico

A interface prototipada no Arduino foi portada para o [STM32F439ZI][stm32]. O display resistivo de 2,4 polegadas, mostrado na Figura 1, usa o controlador [RM68090][rm68090] e comunicação paralela de 8 bits. O driver em C usa o [CMSIS][cmsis] para acessar os periféricos. O [FreeRTOS][freertos] separa o controle do sinal, o diagnóstico serial e a interface em três tarefas.

<p align="center">
  <img src="../images/Etapa03_Firmware_PlataformaIntegrada.png" alt="Placa NUCLEO-F439ZI com o display do gerador em funcionamento" width="300">
</p>
<p align="center"><em>Figura 1 — Placa NUCLEO-F439ZI com o display touchscreen.</em></p>

O firmware lê o toque pelo ADC1, restaura os pinos compartilhados com o LCD e redesenha a tela quando necessário. O diagnóstico usa a porta virtual do ST-LINK pela USART3 a 115200 bit/s.

A Figura 2 mostra a tela principal, com as quatro formas de onda e os campos de ajuste. A seleção da forma de onda é imediata. O usuário pode ajustar cada parâmetro pelo controle deslizante ou pelos botões **+** e **−**. **Salvar** confirma o novo valor e **Cancelar** preserva o anterior. O duty cycle só pode ser ajustado para a onda quadrada.

<p align="center">
  <img src="../images/Etapa03_Interface_TelaInicial.png" alt="Tela principal com as quatro formas de onda e os campos de frequência, amplitude, offset e duty cycle" width="380">
</p>
<p align="center"><em>Figura 2 — Tela principal do gerador com a senoide selecionada.</em></p>

O DAC1 gera a senoide e a onda quadrada em PA4, com temporização pelo TIM6. Conforme a frequência, o firmware seleciona 250, 125, 50, 25 ou 10 pontos por período para a senoide. O teto de 200 kS/s foi definido para a implementação por interrupção, sem DMA. Para triangular e dente de serra, o DAC fornece aos circuitos de conversão uma onda quadrada com duty cycle de 50%.

O firmware aceita de 1 Hz a 20 kHz e indica até 9,8 Vpp com ganho externo conservador de 3,5. O offset selecionado é armazenado, mas ainda não atua na saída. A arquitetura e a pinagem constam na [documentação de software](./software/README.md).

## Circuitos analógicos

Os ensaios da Etapa 2 motivaram caminhos separados para a senoide e as demais formas de onda. O integrador ativo com [TL084][tl084] usa uma referência de 1,65 V, no centro da excursão do DAC, para produzir rampas ascendentes e descendentes. O multiplexador [CD4051B][cd4051b] alterna entre rampas da triangular original e da invertida para formar o dente de serra. A montagem da Figura 3 foi ensaiada com um gerador de laboratório.

<p align="center">
  <img src="../images/Etapa03_GeradorTriangDenteDeSerra_Bancada.jpeg" alt="Protótipo analógico das ondas triangular e dente de serra" width="380">
</p>
<p align="center"><em>Figura 3 — Montagem em bancada do integrador, inversor e multiplexador.</em></p>

Uma única malha RC não cobre a faixa pretendida. O [dimensionamento do integrador](./hardware/dimensionamento_integrador.md) prevê 32 combinações resistivas, selecionadas por quatro multiplexadores. A cobertura da faixa ainda não foi verificada em bancada.

O estágio de saída amplifica o sinal e soma um offset. O ganho teórico é 3,6. Com uma senoide de bancada de 2,08 Vpp a 4,5 kHz, a saída ficou em 6,56 Vpp nos três ajustes de offset, com ganho de 3,15. A Figura 4 compara os casos negativo e positivo.

<p align="center"><img src="../images/Etapa03_OffsetAmplificador_OffsetNegativo.jpeg" alt="Saída azul com offset negativo" width="300"> <img src="../images/Etapa03_OffsetAmplificador_OffsetPositivo.jpeg" alt="Saída azul com offset positivo" width="300"></p>
<p align="center"><em>Figura 4 — Entrada em amarelo e saída em azul. Offset negativo à esquerda e positivo à direita.</em></p>

O capacitor de 82 nF, com resistência equivalente de 46 kΩ, dá um corte próximo de 42 Hz, acima do limite pretendido de 1 Hz. Os circuitos e as simulações constam na [documentação de hardware](./hardware/README.md).

## Protótipo de PCI do circuito analógico com o MCU

O [esquemático](./hardware/WaveFormGenerator/GeradorDeOnda.kicad_sch) no [KiCad][kicad] tem seis folhas: `Supply`, `MCU`, `SineFilter`, `TriangularSawtooth`, `Offset_Amplifier` e `Mux`.

O terra comum do módulo XL6019 avaliado era incompatível com a ligação prevista para obter −12 V. Devido a essa não conformidade e à complexidade da malha de 32 resistores, o grupo decidiu adiar a fabricação da PCI definitiva até a Etapa 4 para incorporar as correções ao projeto.

## Protótipo do gabinete

O gabinete tem painel removível, abertura para o display, entrada de alimentação e saída BNC. O painel tem seis pontos de fixação para resistir à pressão do toque. A Figura 5 mostra o corpo e o painel impressos separadamente para conferir dimensões, encaixe e alinhamento.

<p align="center">
  <img src="../images/Etapa03_Gabinete_Impresso.png" alt="Gabinete do gerador impresso em 3D" width="380">
</p>
<p align="center"><em>Figura 5 — Corpo e painel frontal do gabinete após a impressão 3D.</em></p>

## Testes

Na validação da interface, o grupo navegou sem travamentos observados, ajustou a frequência e a amplitude durante a execução e conferiu o comportamento de **Salvar** e **Cancelar**.

O protótipo de senoide fixa de 1 kHz apresentou aproximadamente 2,2 Vpp. A Figura 6 mostra outra medição, com 999,0 Hz e 2,82 Vpp no osciloscópio.

<p align="center">
  <img src="../images/Etapa03_Firmware_Senoide1kHz.jpeg" alt="Osciloscópio com senoide de 999,0 Hz e 2,82 Vpp" width="380">
</p>
<p align="center"><em>Figura 6 — Senoide de 1 kHz observada no osciloscópio.</em></p>

O filtro senoidal foi ensaiado na faixa de 1 Hz a 20 kHz com o DAC do STM32 na entrada. A Figura 7 mostra o registro a 20 kHz. O filtro suavizou os degraus e a amplitude caiu de 2,88 para 2,08 Vpp. Em outro ensaio, a onda quadrada do DAC apresentou 1,000 kHz e 2,92 Vpp.

<p align="center">
  <img src="../images/Etapa03_FiltroSenoidal_20kHz.png" alt="Senoide de 20 kHz com saída do DAC em amarelo e saída do filtro em azul" width="380">
</p>
<p align="center"><em>Figura 7 — Senoide a 20 kHz. Amarelo indica a saída do DAC e azul indica a saída do filtro.</em></p>

Com o gerador de laboratório, a triangular ficou bem definida a cerca de 15 Hz, como mostra a Figura 8. O dente de serra ficou bem definido a cerca de 30 Hz, como mostra a Figura 9. Em frequências mais altas, como 20 kHz, as duas formas apresentaram distorção na montagem ensaiada.

<p align="center"><img src="../images/Etapa03_OndaTriangular.jpeg" alt="Onda triangular no osciloscópio, em cerca de 15 Hz" width="380"></p>
<p align="center"><em>Figura 8 — Onda triangular medida na saída do integrador.</em></p>

<p align="center"><img src="../images/Etapa03_OndaDenteDeSerra01.jpeg" alt="Onda dente de serra no osciloscópio, em cerca de 30 Hz" width="380"></p>
<p align="center"><em>Figura 9 — Onda dente de serra medida na saída do multiplexador.</em></p>

O firmware foi compilado, gravado e verificado durante os ensaios.

## Referências (links/datasheets/livros)

- [STMicroelectronics — STM32F439ZI][stm32]
- [Raydium — RM68090 LCD Controller][rm68090]
- [Arm — CMSIS][cmsis]
- [FreeRTOS Kernel — documentação oficial][freertos]
- [Texas Instruments — TL084][tl084]
- [Texas Instruments — CD4051B][cd4051b]
- [KiCad — documentação oficial][kicad]

[stm32]: https://www.st.com/resource/en/datasheet/stm32f439zi.pdf
[rm68090]: https://www.crystalfontz.com/controllers/uploaded/Raydium_RM68090_v0.4.pdf
[cmsis]: https://arm-software.github.io/CMSIS_6/latest/General/index.html
[freertos]: https://www.freertos.org/Documentation/02-Kernel/01-About-the-FreeRTOS-kernel/01-About-the-FreeRTOS-Kernel
[tl084]: https://www.ti.com/lit/ds/symlink/tl084.pdf
[cd4051b]: https://www.ti.com/lit/ds/symlink/cd4051b.pdf
[kicad]: https://docs.kicad.org/
