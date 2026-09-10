# Etapa 01

## Estrutura do Circuito
Na imagem a seguir é possível visualizar o Diagrama de Blocos do circuito.
![Diagrama de Blocos](../images/DiagramaDeBlocos_Etapa01.png)

**Descrição:**
*   **STM32G474:** Microcontrolador responsável por gerar os pontos da onda e o sinal de offset via DAC, gerenciar o DMA e controlar a interface.
*   **Display:** Composta por um display touchscreen para configuração dos parâmetros da onda (frequência, amplitude, offset, e forma de onda).
*   **Amplificador Diferencial:** Recebe os sinais de dois DACs para gerar a tensão de offset.
*   **Buffer:** Amplifica a tensão e aplica o offset.
*   **Filtros:** Suavizam a variação discreta gerada pelo DAC.
*   **HLK-PM12:** Fonte chaveada isolada da Hi-Link para saída de +12V/250mA a partir da rede elétrica.
*   **XL6019:** Conversor DC-DC Buck-Boost que gera -12V a partir dos +12V, utilizado na alimentação assimétrica dos AmpOps.
*   **LM2596:** Gera os +5V de alimentação do MCU e do display.

## 3. Requisitos e Seleção do Microcontrolador (MCU)

### 3.1. Requisitos Técnicos
*   Mínimo de 3 canais de DAC internos.
*   Pelo menos um dos DACs com settling time menor do que 5us (para obter pelo menos 10 pontos por período em 20kHz).
*   Controlador DMA para atualização contínua do DAC (mínimo de 200 kSPS).
*   Interface SPI para o display.
*   Suporte a modos de baixo consumo de energia.

### 3.2. Análise Comparativa

| Microcontrolador | 3 DACs | Settling Time | DMA | Avaliação |
| :--- | :---: | :---: | :---: | :--- |
| **ESP32** | Apenas 2 | Não | Sim | Descartado. Sem necessidade de rede; e alto settling time |
| **Raspberry Pi RP2350** | Não possui | Não | Sim | Descartado. Exigiria DAC externo. |
| **MSP430** | Sim | Sim | Não | Descartado. Falta de DMA e frequência baixa (24MHz) para a taxa de atualização necessária (200kHz). |
| **STM32G474** | Sim | Sim | Sim | Selecionado. |

### 3.3. Conclusão da Seleção
O **STM32G474** foi selecionado por atender todos os requisitos apresentados.
