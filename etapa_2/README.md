# Etapa 2

Nesta etapa foram iniciados os testes práticos do gerador de funções, com foco na validação do display touchscreen e no protótipo da interface de usuário. A geração real do sinal ainda não foi implementada; o objetivo foi verificar hardware de interface, leitura do toque, layout e navegação antes da migração para o STM32.

## Desenvolvimento

### 1. Display touchscreen

Foram testados dois shields TFT touchscreen de 2,4" em um Arduino UNO R3 usando as bibliotecas MCUFRIEND_kbv, Adafruit GFX e Adafruit TouchScreen. Os dois módulos funcionaram corretamente, mas o display com controlador **RM68090 (`0x6809`)** foi escolhido como display principal do projeto por ter suporte mais consolidado e documentação mais adequada. O display **LGDP4532 (`0x4532`)** ficou como reserva e foi usado no protótipo de interface. Os detalhes de calibração, seleção e testes estão em [hardware/display.md](./hardware/display.md).

### 2. Protótipo da interface

Foi criado um protótipo visual em Arduino para validar a ergonomia da interface antes da integração com o STM32. A tela permite selecionar senoide, quadrada, triangular ou dente de serra e ajustar frequência, amplitude, offset e duty cycle por telas dedicadas com barra deslizante, botões de incremento/decremento e opções de salvar ou cancelar. O código do protótipo está em [software/interface_arduino/interface_arduino.ino](./software/interface_arduino/interface_arduino.ino), e a descrição da parte de software está em [software/README.md](./software/README.md).

## Testes

Os testes validaram a inicialização dos displays, a leitura do touchscreen resistivo, a calibração dos dois módulos e a navegação básica da interface. O protótipo ainda não gera sinal real; ele serve para avaliar layout, resposta ao toque e fluxo de ajuste dos parâmetros.

## Referências

1. [MCUFRIEND_kbv](https://github.com/prenticedavid/MCUFRIEND_kbv)
2. [Adafruit GFX Library](https://github.com/adafruit/Adafruit-GFX-Library)
3. [Adafruit TouchScreen](https://github.com/adafruit/Adafruit_TouchScreen)
4. [Datasheet do RM68090](https://www.crystalfontz.com/controllers/uploaded/Raydium_RM68090_v0.4.pdf)
