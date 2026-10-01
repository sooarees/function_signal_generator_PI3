# Testes e seleção do display

Foram avaliados dois shields TFT touchscreen de 2,4", com resolução de 240 × 320 e interface paralela de 8 bits. Os testes foram realizados em uma placa Arduino UNO R3 para validar os módulos antes da integração com o microcontrolador do gerador de funções.

## Testes realizados

Foram utilizadas as bibliotecas MCUFRIEND_kbv, Adafruit GFX e Adafruit TouchScreen. O exemplo gráfico validou inicialização, cores, linhas, textos, rotação e rolagem. Em seguida, o touchscreen resistivo foi calibrado e testado com botões virtuais.

| Display | Controlador | Calibração do touch | Resultado |
|:--|:-:|:-:|:-:|
| 1 | RM68090 (`0x6809`) | `LEFT=144`, `RT=883`, `TOP=109`, `BOT=870` | Aprovado |
| 2 | LGDP4532 (`0x4532`) | `LEFT=186`, `RT=889`, `TOP=186`, `BOT=876` | Aprovado |

O LGDP4532 exigiu a ativação manual de seu suporte na biblioteca MCUFRIEND_kbv. Após essa alteração, os testes também funcionaram normalmente.

## Display selecionado

O display com controlador **RM68090** foi selecionado para o gerador de funções. A interface será usada para escolher a forma de onda, ajustar frequência, amplitude e offset, mostrar os valores configurados e apresentar uma prévia do sinal.

A escolha considera principalmente a documentação mais completa e a implementação de referência mais consolidada do RM68090. O controlador oferece escrita paralela de 8 bits e atualização por regiões da memória gráfica, permitindo redesenhar apenas os campos e gráficos alterados. Isso reduz o trabalho da interface e facilita manter sua execução separada das tarefas críticas de geração do sinal.

O LGDP4532 permanecerá disponível como módulo reserva.

## Protótipo da interface no Arduino

Para validar a subentrega da interface sem depender ainda da placa STM32, foi desenvolvido um protótipo funcional no **Arduino UNO R3** usando o display reserva **LGDP4532 (`0x4532`)**. A escolha do módulo reserva evita desgaste do display principal e permite testar livremente o layout, o touchscreen e a navegação antes da migração para o hardware final.

O protótipo usa as bibliotecas **MCUFRIEND_kbv**, **Adafruit GFX** e **Adafruit TouchScreen**, em orientação paisagem (`320 × 240`). A tela inicial possui quatro botões grandes com o desenho de um período das formas de onda: senoidal, quadrada, triangular e dente de serra. Abaixo ficam os botões de parâmetros: frequência, amplitude, offset e duty cycle. Ao tocar em um parâmetro, a interface abre uma tela dedicada com o valor centralizado, uma barra de ajuste, botões de incremento e decremento, além das opções de cancelar ou salvar.

Os limites adotados no protótipo seguem os requisitos definidos na etapa 1: frequência de **1 Hz a 20 kHz**, amplitude de **0 a 10 Vpp**, offset de **−5 V a +5 V** e duty cycle de **0% a 100%**. A interface não gera o sinal real neste momento; seu objetivo é validar a ergonomia, a leitura do toque, a organização das telas e o comportamento dos controles.

O estilo visual atual usa fundo escuro com linhas azuis em uma proposta inspirada em interfaces do tipo **Tron**, mantendo alto contraste para facilitar a leitura no display TFT. Como o Arduino UNO possui pouca memória, o desenho é feito diretamente na tela, sem framebuffer, e as atualizações são concentradas nas regiões alteradas.

## Registros dos testes

As fotos e os vídeos dos testes serão adicionados posteriormente.

## Referências

- [MCUFRIEND_kbv](https://github.com/prenticedavid/MCUFRIEND_kbv)
- [Datasheet do RM68090](https://www.crystalfontz.com/controllers/uploaded/Raydium_RM68090_v0.4.pdf)
