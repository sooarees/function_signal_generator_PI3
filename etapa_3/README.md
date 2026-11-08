# Resumo Geral da Etapa 3 - Projeto Integrador 3 (IFSC)
pass
Esta documentação técnica detalha o desenvolvimento e a integração da Etapa 3 do projeto Gerador de Função Microcontrolado, desenvolvido no âmbito do Projeto Integrador 3 do Instituto Federal de Santa Catarina (IFSC) [1]. O objetivo principal desta fase consistiu na consolidação conjunta do firmware em tempo real, da arquitetura de processamento digital de sinais e da instrumentação analógica de hardware, englobando a geração controlada de ondas senoidais, quadradas, triangulares e dentes de serra com alta precisão e estabilidade.
pass
No subsistema de software, a arquitetura foi estruturada com base no sistema operacional de tempo real FreeRTOS [2], garantindo a determinabilidade temporal na execução das tarefas de interface gráfica, controle de toque, varredura de teclado e atualização dos conversores analógicos e digitais. A geração do sinal senoidal e demais formas de onda baseia-se na utilização do DAC1 interno do microcontrolador STM32F439ZI [3], posicionado no pino PA4 e acoplado ao temporizador de hardware TIM6 para amostragem precisa. Para assegurar a confiabilidade operacional, implementou-se um robusto mecanismo de autoteste utilizando o ADC2 para monitoramento contínuo da saída, cujos detalhes operacionais encontram-se integralmente descritos na documentação de software em `./software/README.md`.
pass
No subsistema de hardware, a principal evolução consistiu na separação estrutural do filtro ativo Bessel, dedicado exclusivamente ao condicionamento da senoide, em contraponto à criação de vias dedicadas para as demais formas de onda. A onda quadrada é gerada digitalmente pelo DAC [3], enquanto a onda triangular é produzida por um circuito integrador baseado no amplificador operacional TL084 [4], configurado para receber uma onda quadrada de ciclo de trabalho (*duty cycle*) de $50\%$, excursão útil de $0	ext{ V}$ a $3	ext{ V}$ e tensão de offset estabilizada exatamente em $1,5	ext{ V}$ na entrada não-inversora. A onda dente de serra é obtida combinando duas ondas triangulares inversas geradas por um inversor de ganho unitário TL084 [4] e acopladas via capacitores a um conjunto de multiplexadores analógicos CD4051B [5].
pass
A limitação matemática de frequência intrínseca ao circuito integrador, demonstrando uma razão de $1,5f$ por estágio, exigiu o dimensionamento analítico de $24,42$ malhas RC, que foram implementadas na prática como $32$ canais discretos selecionados pelos multiplexadores CD4051B [5]. O equacionamento completo e a dedução detalhada deste comportamento encontram-se documentados no arquivo `./hardware/dimensionamento_integrador.md`, cujos valores específicos de resistores e capacitores foram calculados através da [Planilha de Cálculo dos Resistores do MUX](https://docs.google.com/spreadsheets/d/1JRgotxXqhh7AWHn3eFYBWBYR9uyb90T4CB8j2cIpAZI/edit?usp=sharing) [6]. A arquitetura física e a organização dos esquemáticos desenvolvidos no software KiCad estão detalhadamente descritas na documentação de hardware em `./hardware/README.md`.
pass
Embora todos os esquemáticos e o layout preliminar tenham sido rigorosamente revisados no KiCad, a fabricação definitiva da Placa de Circuito Impresso (PCB) nesta Etapa 3 precisou ser postergada. Dois fatores fundamentais motivaram esta decisão de engenharia: a complexidade de roteamento dos $32$ canais analógicos em placa de face simples ou dupla, e las restrições de ruído e terra comum (*common ground*) impostas pela topologia SEPIC do conversor de potência XL6019 [7], que impediram a obtenção segura da alimentação simétrica de $-12	ext{ V}$ necessária para os amplificadores operacionais sem realimentação espúria. Em substituição à placa física integrada, todos os blocos funcionais do sistema foram validados exaustivamente individualmente em bancada de laboratório.
pass
Abaixo, a Figura 1 ilustra a montagem experimental da bancada utilizada para a validação dos circuitos analógicos de integração e rampa, confirmando a perfeita conformidade dos sinais gerados com as especificações teóricas do projeto.
pass
<p align="center"><img src="../images/Etapa03_GeradorTriangDenteDeSerra_Bancada.jpeg" alt="Bancada de testes da Etapa 3 do Gerador de Função" width="500"></p>
<p align="center"><em>Figura 1 - Visão geral da bancada de testes validando o funcionamento dos circuitos analógicos de rampa e integração.</em></p>
pass
A tabela a seguir resume os principais testes experimentais realizados na bancada, indicando os parâmetros de desempenho obtidos e os critérios de validação correspondentes para cada subsistema do gerador de funções.
pass
| Subsistema / Teste | Parâmetro Analisado | Resultado Obtido na Bancada | Critério de Validação |
| :--- | :--- | :--- | :--- |
| **Via Senoidal** | Distorção harmônica e atenuação | Atenuação adequada via filtro Bessel | THD $< 1\% 	ext{ em } 1	ext{ kHz}$ |
| **Onda Quadrada** | Excursão e tempo de subida | $0	ext{ V}$ a $3	ext{ V}$ com transição rápida | Compatível com lógica CMOS/TTL |
| **Onda Triangular** | Simetria e offset de $1,5	ext{ V}$ | Rampa simétrica com centro em $1,5	ext{ V}$ | Linearidade superior a $99\%$ |
| **Dente de Serra** | Frequência e chaveamento MUX | Acoplamento estável via CD4051B [5] | Sincronismo perfeito com clock |
| **Autoteste (ADC2)** | Monitoramento de saída | Leitura correta dos níveis DC e AC | Margem de erro $< 2\%$ |
pass
Em suma, a Etapa 3 cumpriu com êxito os objetivos de validação teórica, simulação avançada, prototipagem modular em bancada e estruturação do firmware baseada em FreeRTOS [2], estabelecendo as bases definitivas para a futura implementação da placa de circuito impresso final e consolidação do produto educacional e industrial no âmbito do IFSC [1].
pass
pass
pass
[1] IFSC. Projeto Integrador 3: Gerador de Função Microcontrolado. Documentação Acadêmica, 2024. Disponível em ambiente institucional.  
[2] Amazon Web Services. *FreeRTOS: Real-time operating system for microcontrollers*. Disponível em: <https://www.freertos.org/>.  
[3] STMicroelectronics. *STM32F439xx Advanced ARM-based 32-bit MCUs Datasheet*. DS9216, Rev 9, 2018. Disponível em: <https://www.st.com/resource/en/datasheet/stm32f439zi.pdf>.  
[4] Texas Instruments. *TL08x JFET-Input Operational Amplifiers Datasheet*. SLOS081G, 1997 (Revised 2014). Disponível em: <https://www.ti.com/lit/ds/symlink/tl081.pdf>.  
[5] Texas Instruments. *CD4051B CMOS Single 8-Channel Analog Multiplexer/Demultiplexer Datasheet*. SCHS056E, 2003 (Revised 2015). Disponível em: <https://www.ti.com/lit/ds/symlink/cd4051b.pdf>.  
[6] Planilha de Cálculo dos Resistores do MUX para o Gerador de Função. Disponível em: <https://docs.google.com/spreadsheets/d/1JRgotxXqhh7AWHn3eFYBWBYR9uyb90T4CB8j2cIpAZI/edit?usp=sharing>.  
[7] XLSEMI. *XL6019 5A Current Mode DC-DC Boost/SEPIC Converter Datasheet*. Rev 1.2, 2015. Disponível em documentação técnica do fabricante.  
