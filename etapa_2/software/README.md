# Software

Esta pasta contém um protótipo inicial da interface do gerador de funções para Arduino UNO com o shield TFT touchscreen MCUFRIEND de 2,4".

## Interface Arduino

O sketch está em:

`interface_arduino/interface_arduino.ino`

Objetivos do protótipo:

- validar o layout em modo paisagem;
- testar botões grandes no touchscreen resistivo;
- selecionar senoide, quadrada, triangular ou dente de serra;
- ajustar frequência, amplitude e offset;
- ajustar o duty cycle somente para a onda quadrada;
- redesenhar apenas os valores e a área da forma de onda quando houver alteração.

Este sketch usa o display reserva, com ID `0x4532`, e a calibração de touch correspondente.

## Bibliotecas

- MCUFRIEND_kbv
- Adafruit GFX Library
- Adafruit TouchScreen

## Observação

Neste momento, o código não gera sinal real. Ele serve apenas para validar a interface e sua ergonomia antes da futura migração para o STM32.


