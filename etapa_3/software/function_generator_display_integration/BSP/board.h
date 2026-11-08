#ifndef BOARD_H
#define BOARD_H

#include "stm32f4xx.h"

// pinos de controle do shield mcufriend

#define BOARD_LCD_RD_GPIO            GPIOA
#define BOARD_LCD_RD_PIN             GPIO_ODR_OD3
#define BOARD_LCD_WR_GPIO            GPIOC
#define BOARD_LCD_WR_PIN             GPIO_ODR_OD0
#define BOARD_LCD_RS_GPIO            GPIOC
#define BOARD_LCD_RS_PIN             GPIO_ODR_OD3
#define BOARD_LCD_CS_GPIO            GPIOF
#define BOARD_LCD_CS_PIN             GPIO_ODR_OD3
#define BOARD_LCD_RST_GPIO           GPIOF
#define BOARD_LCD_RST_PIN            GPIO_ODR_OD5

// barramento de dados do lcd

// d0/d2/d4/d7 no gpiof, d1 no gpiod e d3/d5/d6 no gpioe.
#define BOARD_LCD_D0_PIN             GPIO_ODR_OD12
#define BOARD_LCD_D1_PIN             GPIO_ODR_OD15
#define BOARD_LCD_D2_PIN             GPIO_ODR_OD15
#define BOARD_LCD_D3_PIN             GPIO_ODR_OD13
#define BOARD_LCD_D4_PIN             GPIO_ODR_OD14
#define BOARD_LCD_D5_PIN             GPIO_ODR_OD11
#define BOARD_LCD_D6_PIN             GPIO_ODR_OD9
#define BOARD_LCD_D7_PIN             GPIO_ODR_OD13

#define BOARD_LCD_DATA_GPIO_F        GPIOF
#define BOARD_LCD_DATA_GPIO_D        GPIOD
#define BOARD_LCD_DATA_GPIO_E        GPIOE
#define BOARD_LCD_DATA_F_MASK        (BOARD_LCD_D0_PIN | BOARD_LCD_D2_PIN | \
                                      BOARD_LCD_D4_PIN | BOARD_LCD_D7_PIN)
#define BOARD_LCD_DATA_D_MASK        BOARD_LCD_D1_PIN
#define BOARD_LCD_DATA_E_MASK        (BOARD_LCD_D3_PIN | BOARD_LCD_D5_PIN | \
                                      BOARD_LCD_D6_PIN)

#define BOARD_LCD_GPIO_CLOCK_ENABLE  (RCC_AHB1ENR_GPIOAEN | \
                                      RCC_AHB1ENR_GPIOCEN | \
                                      RCC_AHB1ENR_GPIODEN | \
                                      RCC_AHB1ENR_GPIOEEN | \
                                      RCC_AHB1ENR_GPIOFEN)

// touch resistivo compartilhado com o barramento do lcd

#define BOARD_TOUCH_XP_GPIO          GPIOE
#define BOARD_TOUCH_XP_PIN           GPIO_ODR_OD9
#define BOARD_TOUCH_XM_GPIO          GPIOC
#define BOARD_TOUCH_XM_PIN           GPIO_ODR_OD3
#define BOARD_TOUCH_YP_GPIO          GPIOC
#define BOARD_TOUCH_YP_PIN           GPIO_ODR_OD0
#define BOARD_TOUCH_YM_GPIO          GPIOF
#define BOARD_TOUCH_YM_PIN           GPIO_ODR_OD13
#define BOARD_TOUCH_ADC              ADC1
#define BOARD_TOUCH_ADC_CLOCK        RCC_APB2ENR_ADC1EN
#define BOARD_TOUCH_XM_ADC_CHANNEL   13U
#define BOARD_TOUCH_YP_ADC_CHANNEL   10U

// monitor interno de pa4 pelo canal 4 do adc2

#define BOARD_SIGNAL_MONITOR_ADC          ADC2
#define BOARD_SIGNAL_MONITOR_ADC_CLOCK    RCC_APB2ENR_ADC2EN
#define BOARD_SIGNAL_MONITOR_ADC_CHANNEL  4U

// porta serial virtual do st-link pela usart3

#define BOARD_DIAGNOSTIC_UART                 USART3
#define BOARD_DIAGNOSTIC_UART_CLOCK_ENABLE    RCC_APB1ENR_USART3EN
#define BOARD_DIAGNOSTIC_UART_TX_GPIO         GPIOD
#define BOARD_DIAGNOSTIC_UART_TX_GPIO_CLOCK   RCC_AHB1ENR_GPIODEN
#define BOARD_DIAGNOSTIC_UART_TX_PIN_NUMBER   8U
#define BOARD_DIAGNOSTIC_UART_RX_PIN_NUMBER   9U
#define BOARD_DIAGNOSTIC_UART_TX_AF           7U
#define BOARD_DIAGNOSTIC_UART_RX_AF           7U

#endif
