# Software do gerador de funções

Esta pasta documenta o protótipo integrado desenvolvido para a placa
NUCLEO-F439ZI, com STM32F439ZI, FreeRTOS, display resistivo MCUFRIEND e geração
de sinais pelo DAC interno. O projeto usado no STM32CubeIDE 2.2.0 chama-se
`function_generator_display_integration`.

## Estado da versão

A versão final desta etapa foi compilada com `-Wall -Werror`, sem erros ou
avisos, gravada na placa e verificada pelo STM32CubeProgrammer. O firmware ocupa
aproximadamente 35 kB de flash e 18,96 kB de RAM estática, valor que inclui o
heap de 16 kB reservado ao FreeRTOS.

O ELF final possui SHA-256
`916A3B217881A5C6D6AC1F57118A86F7CFBA567521DE509AA49AF838CB353AC3`.
A gravação foi realizada na NUCLEO-F439ZI alimentada com 3,24 V, sem alteração
dos option bytes.

## Arquitetura

O driver `waveform_generator` controla o DAC1 canal 1 e o TIM6. O módulo
`signal_control` mantém o estado oficial do gerador, valida os limites e protege
alterações concorrentes com mutex. A UI mantém somente um rascunho durante a
edição e envia o novo valor ao controle apenas quando o usuário toca em Salvar.
Cancelar descarta o rascunho sem alterar a saída.

O FreeRTOS executa três tasks:

- `SignalControl`, prioridade 5 e stack de 1536 bytes, inicializa e mantém o
  controle central;
- `SerialControl`, prioridade 4 e stack de 1536 bytes, atende somente aos
  comandos de diagnóstico;
- `Interface`, prioridade 1 e stack de 1536 bytes, controla display e touch.

A interrupção do TIM6 usa prioridade 4 e não chama APIs do FreeRTOS. A USART3
usa prioridade 7. Dessa forma, a atualização do DAC tem precedência sobre a
comunicação serial e sobre as tasks da interface.

## Formas de onda e limites

A saída analógica bruta utiliza `PA4`, correspondente a `DAC_OUT1`, disponível
em `D24 / CN7-17`. O GND é comum à placa e aos instrumentos externos.

Os valores aceitos pelo controle são:

- frequência de 1 Hz a 20 kHz;
- amplitude de 0,0 a 9,8 Vpp, considerando o amplificador externo de 3,5 vezes;
- duty cycle de 0% a 100% para a onda quadrada;
- offset armazenado entre -5,0 V e +5,0 V, ainda sem atuação elétrica.

A configuração inicial é senoide, 1 kHz, 1,0 Vpp, offset zero e duty cycle de
50%. A senoide utiliza uma tabela mestra de 250 pontos e seleciona
automaticamente 250, 125, 50, 25 ou 10 pontos para manter a taxa máxima de
atualização em 200 mil amostras por segundo.

A onda quadrada também é produzida pelo DAC. As seleções triangular e dente de
serra são estados lógicos válidos da interface, mas nesta etapa fazem o DAC
produzir uma quadrada com duty fixo em 50%. A conversão para triangular ou dente
de serra será realizada posteriormente pelo circuito analógico externo.

## Display, touch e pinos

O display e o touch foram integrados sem ocupar PA4. O barramento paralelo do
display utiliza GPIOA, GPIOC, GPIOD, GPIOE e GPIOF. O touch resistivo compartilha
alguns desses sinais e usa o ADC1 nos canais 10 e 13. Após cada leitura, o
driver restaura os pinos do barramento do display.

O autoteste usa o ADC2 canal 4 para medir internamente o mesmo nó analógico de
PA4. Portanto, ele verifica a saída bruta do DAC e não a saída do amplificador
externo de até 9,8 Vpp.

## Interface e diagnóstico

O touch é a interface principal. Frequência, amplitude, offset e duty cycle são
aplicados somente pelo botão Salvar. A escolha da forma de onda é aplicada
imediatamente. O controle de duty fica disponível apenas quando a quadrada está
selecionada.

A porta virtual do ST-LINK utiliza USART3 em PD8 e PD9. No computador ela foi
identificada como COM8, configurada em 115200 bit/s, 8 bits, sem paridade, um
stop bit e sem controle de fluxo. O número da COM pode mudar em outro
computador.

Na versão final, a serial não altera configurações. Estão disponíveis somente
os comandos `status`, que mostra o estado oficial, e `selftest`, que mede PA4 e
informa tensão mínima, máxima, média, amplitude, frequência e resultado. O
autoteste não roda automaticamente durante o boot.

## Validações realizadas

Foram confirmados na placa:

- inicialização simultânea de FreeRTOS, display, touch, DAC e USART3;
- funcionamento dos botões Salvar e Cancelar;
- atualização das configurações pelo touch;
- leitura do estado pela serial;
- ausência de conflito entre display, touch e PA4;
- autoteste de uma senoide de 1 kHz e 1,0 Vpp.

No autoteste registrado, o ADC2 mediu 1,444 V de mínimo, 1,749 V de máximo,
aproximadamente 1,595 V de média, 305 mVpp em PA4 e 1 kHz. O resultado foi
classificado como válido pelo firmware.

O projeto mantém hooks para falha de alocação, overflow de stack e assert. Em
qualquer uma dessas falhas, o gerador desliga o timer e deixa o DAC no valor de
offset antes de interromper a execução. A revisão final também conferiu heap,
stacks, prioridades, pinos e limites.

## Trabalho futuro

Os testes completos entre 1 Hz e 20 kHz foram adiados. Também permanecem para
etapas futuras a implementação elétrica do offset, os circuitos de conversão
para triangular e dente de serra, a validação do amplificador externo, a
migração da senoide para DMA e a medição de jitter, ruído e distorção com
osciloscópio.

## Referências

- [STM32F439ZI — página oficial](https://www.st.com/en/microcontrollers-microprocessors/stm32f439zi.html)
- [Datasheet do STM32F439ZI](https://www.st.com/resource/en/datasheet/stm32f439zi.pdf)
- [Manual das placas Nucleo-144 UM1974](https://www.st.com/resource/en/user_manual/um1974-stm32-nucleo-144-boards-mb1137-stmicroelectronics.pdf)
- [FreeRTOS Kernel](https://www.freertos.org/Documentation/02-Kernel/01-About-the-FreeRTOS-kernel/01-About-the-FreeRTOS-Kernel)
