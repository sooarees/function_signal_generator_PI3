# Hardware

Na Etapa 2, os testes de hardware se concentraram no display touchscreen e no filtro de reconstrução da saída analógica.

## Display touchscreen

Dois shields TFT de 2,4" foram testados e aprovados. O módulo com controlador RM68090 [1] foi escolhido como principal, enquanto o LGDP4532 ficou como reserva e foi usado no protótipo da interface. Os resultados e valores de calibração estão em [Testes e seleção do display](./display.md).

## Filtro de reconstrução

Foi simulado e montado um filtro passa-baixa com aproximação de Bessel [2]. O protótipo não apresentou em bancada o mesmo resultado obtido na simulação, principalmente para a onda quadrada. Por isso, o circuito será revisto e a onda quadrada seguirá por um caminho separado das demais formas de onda.

## Referências

1. [Datasheet do RM68090](https://www.crystalfontz.com/controllers/uploaded/Raydium_RM68090_v0.4.pdf)
2. [Analog Devices — fundamentos de DDS e filtros de reconstrução](https://www.analog.com/media/en/training-seminars/design-handbooks/Technical-Tutorial-DDS/Section4.pdf)


