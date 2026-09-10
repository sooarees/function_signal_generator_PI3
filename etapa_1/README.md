# Etapa 1

## Desenvolvimento

### Definição dos parâmetros ajustáveis

| Parâmetro | Faixa | Implementação |
|---|---|---|
| Forma de onda | senoidal, triangular, dente de serra, retangular | tabela + chave |
| Frequência | 1 Hz a 20 kHz | passo do acumulador |
| Amplitude | 0 a 10 Vpp | escalamento digital + atenuador |
| Offset | −5 V a +5 V | conversor interno |
| Simetria | 5% a 95% | parâmetro da tabela |
| Ciclo de trabalho | 10% a 90% | temporizador |

### Método de síntese: DDS

**DDS é a técnica; o DAC é um componente dentro dela.** A síntese reúne três partes: **acumulador de fase** de 32 bits em firmware, **tabela de onda** de 16.384 pontos e o **conversor D/A** interno (DAC3_CH1).

```
acumulador += passo        ← define a frequência
índice = acumulador >> 18  ← 14 bits superiores
valor  = tabela[índice]    ← DMA entrega ao conversor
```

O acumulador determina **a velocidade**; a tabela, **o formato**. Serão duas: senoidal e rampa com simetria ajustável, que gera triangular em 50% e dente de serra nos extremos. Com buffer duplo, 64 kB.

**A retangular é exceção:** vem de um temporizador, sem tabela nem conversor, pois o filtro arredondaria as bordas que a definem.

### Escolha do microcontrolador

A carga de 9 MHz contínuos é atendida por qualquer núcleo de 32 bits acima de 100 MHz: **desempenho não é critério discriminante**. Adotou-se a integração analógica como critério.

| Parâmetro | ATmega | dsPIC33CK | MSP430FG | ESP32-S3 | RP2350 | STM32G474 |
|---|---|---|---|---|---|---|
| Ocupação de CPU | 190% | 17% | 20% | 4% | 3% | 5,3% |
| SRAM | 2–16 kB | 24 kB | 8 kB | 512 kB | 520 kB | 128 kB |
| Canais D/A < 667 ns | 0 | 0 | 0 | 0 | 0 | **4** |
| Amplificadores | 0–3 | 3 | 3 | 0 | 0 | **6** |
| CIs analógicos externos | — | — | — | ~8 | ~8 | **3** |

O **ATmega** é inviável por carga, falta de DMA e memória. O **dsPIC33CK** tem periferia competitiva, mas seus 24 kB reduziriam a tabela, elevando os espúrios a −72 dBc. No **MSP430FG**, a acomodação do DAC12 é de microssegundos, limitando a dente de serra a 1,7 kHz. **ESP32-S3** e **RP2350** atendem à vazão, sem periferia analógica.

O **STM32G474** é o único a satisfazer o critério. Além da conversão, absorve offset, condicionamento da retangular e referência, resultando em três CIs externos contra cerca de oito.

## Testes

1. **Alimentação.** Ondulação nos trilhos abaixo de 1 mVpp em 120 Hz.
2. **Cadeia digital.** Barramento com analisador lógico; 1,5 MS/s confirmados.
3. **Conversão.** Degraus no osciloscópio; acomodação conversor–amplificador.
4. **Filtro.** Corte em 300 kHz; rejeição de 69 dB em 1,48 MHz.
5. **Espectro.** Acoplamento do barramento, que contorna o filtro.
6. **Formas de onda.** Retorno da dente de serra; subida da retangular.
7. **Amplitude mínima.** 100 mVpp: distorção de cruzamento.
8. **Proteção.** Curto prolongado e tensão externa na saída.

## Referências

- ST. *STM32G474xB/xC/xE Datasheet*, DS12288.
- ST. *RM0440 — STM32G4 Series Reference Manual*.
- ST. *AN5306 — Operational Amplifier Usage in STM32G4*.
- TI. *MSP430FG461x Mixed-Signal Microcontrollers*, SLAS508.
- KESTER, W. *The Data Conversion Handbook*. ADI, 2005.
- HOROWITZ, P.; HILL, W. *The Art of Electronics*. Cambridge, 2015.
- OPPENHEIM, A. V.; SCHAFER, R. W. *Discrete-Time Signal Processing*. Pearson, 2009.
- ADI. *Technical Tutorial on Digital Signal Synthesis*, 1999.
