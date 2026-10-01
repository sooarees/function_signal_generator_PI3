# Software

Esta pasta contem um prototipo inicial da interface do gerador de funcoes para Arduino UNO com o shield TFT touchscreen MCUFRIEND 2.4".

## Interface Arduino

O sketch esta em:

`interface_arduino/interface_arduino.ino`

Objetivo do prototipo:

- validar o layout em paisagem;
- testar botoes grandes no touchscreen resistivo;
- selecionar senoide, quadrada, triangular ou dente de serra;
- ajustar frequencia, amplitude, offset e duty cycle;
- redesenhar apenas os valores e a area da forma de onda quando houver alteracao.

Este sketch usa o display reserva, com ID `0x4532`, e a calibracao de touch correspondente.

## Bibliotecas

- MCUFRIEND_kbv
- Adafruit GFX Library
- Adafruit TouchScreen

## Observacao

Neste momento o codigo nao gera sinal real. Ele serve apenas para validar a subentrega de interface e ergonomia antes da futura migracao para STM32.


