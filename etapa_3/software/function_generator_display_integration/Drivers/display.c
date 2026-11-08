#include "board.h"
#include "display.h"
#include "font5x7.h"

#if DISPLAY_CONTROLLER_ID != 0x6809U
#error "This driver supports only the RM68090 controller (ID 0x6809)."
#endif

typedef struct
{
	uint16_t register_address;
	uint16_t value;
	uint16_t delay_ms;
} display_init_command_t;

// sequencia de inicializacao do controlador

static const display_init_command_t init_sequence[] = {
	{0x00E5U, 0x78F0U, 0U}, {0x0001U, 0x0100U, 0U},
	{0x0002U, 0x0200U, 0U}, {0x0003U, 0x1030U, 0U},
	{0x0004U, 0x0000U, 0U}, {0x0005U, 0x0000U, 0U},
	{0x0008U, 0x0207U, 0U}, {0x0009U, 0x0000U, 0U},
	{0x000AU, 0x0000U, 0U}, {0x000CU, 0x0000U, 0U},
	{0x000DU, 0x0000U, 0U}, {0x000FU, 0x0000U, 0U},
	{0x0010U, 0x0000U, 0U}, {0x0011U, 0x0007U, 0U},
	{0x0012U, 0x0000U, 0U}, {0x0013U, 0x0000U, 0U},
	{0x0007U, 0x0001U, 200U}, {0x0010U, 0x1690U, 0U},
	{0x0011U, 0x0227U, 50U}, {0x0012U, 0x000DU, 50U},
	{0x0013U, 0x1200U, 0U}, {0x0029U, 0x000AU, 0U},
	{0x002BU, 0x000DU, 50U}, {0x0020U, 0x0000U, 0U},
	{0x0021U, 0x0000U, 0U}, {0x0030U, 0x0000U, 0U},
	{0x0031U, 0x0404U, 0U}, {0x0032U, 0x0003U, 0U},
	{0x0035U, 0x0405U, 0U}, {0x0036U, 0x0808U, 0U},
	{0x0037U, 0x0407U, 0U}, {0x0038U, 0x0303U, 0U},
	{0x0039U, 0x0707U, 0U}, {0x003CU, 0x0504U, 0U},
	{0x003DU, 0x0808U, 0U}, {0x0060U, 0x2700U, 0U},
	{0x0061U, 0x0001U, 0U}, {0x006AU, 0x0000U, 0U},
	{0x0080U, 0x0000U, 0U}, {0x0081U, 0x0000U, 0U},
	{0x0082U, 0x0000U, 0U}, {0x0083U, 0x0000U, 0U},
	{0x0084U, 0x0000U, 0U}, {0x0085U, 0x0000U, 0U},
	{0x0090U, 0x0010U, 0U}, {0x0092U, 0x0000U, 0U},
	{0x0007U, 0x0133U, 0U}
};

// acesso aos pinos

static void gpio_write_high(GPIO_TypeDef *gpio, uint32_t pin)
{
	gpio->BSRR = pin;
}

static void gpio_write_low(GPIO_TypeDef *gpio, uint32_t pin)
{
	gpio->BSRR = pin << 16U;
}

static void gpio_configure_output(GPIO_TypeDef *gpio, uint32_t pins)
{
	uint32_t pin_number;

	for (pin_number = 0U; pin_number < 16U; pin_number++)
	{
		const uint32_t pin = 1UL << pin_number;

		if ((pins & pin) != 0U)
		{
			const uint32_t shift = pin_number * 2U;
			const uint32_t mask = 3UL << shift;

			MODIFY_REG(gpio->MODER, mask, 1UL << shift);
			CLEAR_BIT(gpio->OTYPER, pin);
			MODIFY_REG(gpio->OSPEEDR, mask, 1UL << shift);
			CLEAR_BIT(gpio->PUPDR, mask);
		}
	}
}

static void display_set_touch_idle_levels(void)
{
	// estes quatro sinais do lcd tambem sao os eletrodos do touch.
	// o mesmo nivel evita corrente continua entre uma leitura e outra.
	gpio_write_high(BOARD_LCD_WR_GPIO, BOARD_LCD_WR_PIN);
	gpio_write_high(BOARD_LCD_RS_GPIO, BOARD_LCD_RS_PIN);
	gpio_write_high(BOARD_TOUCH_XP_GPIO, BOARD_TOUCH_XP_PIN);
	gpio_write_high(BOARD_TOUCH_YM_GPIO, BOARD_TOUCH_YM_PIN);
}

// comunicacao com o controlador

static void display_delay_ms(uint32_t milliseconds)
{
	const uint32_t cycles_per_ms = SystemCoreClock / 1000U;

	while (milliseconds > 0U)
	{
		const uint32_t start = DWT->CYCCNT;

		while ((DWT->CYCCNT - start) < cycles_per_ms)
		{
		}
		milliseconds--;
	}
}

static void display_write_bus(uint8_t value)
{
	uint32_t value_f = 0U;
	uint32_t value_d = 0U;
	uint32_t value_e = 0U;

	if ((value & (1U << 0U)) != 0U) value_f |= BOARD_LCD_D0_PIN;
	if ((value & (1U << 1U)) != 0U) value_d |= BOARD_LCD_D1_PIN;
	if ((value & (1U << 2U)) != 0U) value_f |= BOARD_LCD_D2_PIN;
	if ((value & (1U << 3U)) != 0U) value_e |= BOARD_LCD_D3_PIN;
	if ((value & (1U << 4U)) != 0U) value_f |= BOARD_LCD_D4_PIN;
	if ((value & (1U << 5U)) != 0U) value_e |= BOARD_LCD_D5_PIN;
	if ((value & (1U << 6U)) != 0U) value_e |= BOARD_LCD_D6_PIN;
	if ((value & (1U << 7U)) != 0U) value_f |= BOARD_LCD_D7_PIN;

	BOARD_LCD_DATA_GPIO_F->BSRR = (BOARD_LCD_DATA_F_MASK << 16U) | value_f;
	BOARD_LCD_DATA_GPIO_D->BSRR = (BOARD_LCD_DATA_D_MASK << 16U) | value_d;
	BOARD_LCD_DATA_GPIO_E->BSRR = (BOARD_LCD_DATA_E_MASK << 16U) | value_e;
	__DSB();
}

static void display_write8(uint8_t value)
{
	display_write_bus(value);
	gpio_write_low(BOARD_LCD_WR_GPIO, BOARD_LCD_WR_PIN);
	__NOP();
	__NOP();
	gpio_write_high(BOARD_LCD_WR_GPIO, BOARD_LCD_WR_PIN);
}

static void display_write16(uint16_t value)
{
	display_write8((uint8_t)(value >> 8U));
	display_write8((uint8_t)value);
}

static void display_write_register(uint16_t register_address, uint16_t value)
{
	gpio_write_low(BOARD_LCD_CS_GPIO, BOARD_LCD_CS_PIN);
	gpio_write_low(BOARD_LCD_RS_GPIO, BOARD_LCD_RS_PIN);
	display_write16(register_address);
	gpio_write_high(BOARD_LCD_RS_GPIO, BOARD_LCD_RS_PIN);
	display_write16(value);
	gpio_write_high(BOARD_LCD_CS_GPIO, BOARD_LCD_CS_PIN);
	display_set_touch_idle_levels();
}

static void display_set_window(uint16_t x, uint16_t y,
						   uint16_t width, uint16_t height)
{
	const uint16_t x_end = (uint16_t)(x + width - 1U);
	const uint16_t y_end = (uint16_t)(y + height - 1U);

	// a rotacao horizontal troca os eixos da janela do controlador.
	display_write_register(0x0052U, x);
	display_write_register(0x0053U, x_end);
	display_write_register(0x0050U, y);
	display_write_register(0x0051U, y_end);
	display_write_register(0x0021U, x);
	display_write_register(0x0020U, y);
}

static void display_begin_pixels(uint16_t x, uint16_t y,
							 uint16_t width, uint16_t height)
{
	display_set_window(x, y, width, height);
	gpio_write_low(BOARD_LCD_CS_GPIO, BOARD_LCD_CS_PIN);
	gpio_write_low(BOARD_LCD_RS_GPIO, BOARD_LCD_RS_PIN);
	display_write16(0x0022U);
	gpio_write_high(BOARD_LCD_RS_GPIO, BOARD_LCD_RS_PIN);
}

static void display_end_pixels(void)
{
	gpio_write_high(BOARD_LCD_CS_GPIO, BOARD_LCD_CS_PIN);
	display_set_touch_idle_levels();
}

// desenho de caracteres

static int16_t display_abs(int16_t value)
{
	return value < 0 ? (int16_t)-value : value;
}

static uint8_t display_font_column(uint8_t character, uint8_t column)
{
	if ((character < 32U) || (character > 126U) || (column >= 5U))
	{
		return 0U;
	}
	return font[((uint16_t)character * 5U) + column];
}

static void display_draw_char(int16_t x, int16_t y, char character,
							  uint16_t foreground, uint16_t background,
							  uint8_t size)
{
	uint8_t row;

	for (row = 0U; row < 8U; row++)
	{
		uint8_t column = 0U;

		while (column < 6U)
		{
			const uint8_t bits = display_font_column((uint8_t)character, column);
			const uint16_t color = ((bits & (1U << row)) != 0U)
				? foreground : background;
			uint8_t run = 1U;

			while ((uint8_t)(column + run) < 6U)
			{
				const uint8_t next_bits = display_font_column(
					(uint8_t)character, (uint8_t)(column + run));
				const uint16_t next_color = ((next_bits & (1U << row)) != 0U)
					? foreground : background;
				if (next_color != color)
				{
					break;
				}
				run++;
			}

			display_fill_rect((int16_t)(x + (column * size)),
							  (int16_t)(y + (row * size)),
							  (int16_t)(run * size), size, color);
			column = (uint8_t)(column + run);
		}
	}
}

// inicializacao do display

void display_init(void)
{
	uint32_t command_index;

	SET_BIT(RCC->AHB1ENR, BOARD_LCD_GPIO_CLOCK_ENABLE);
	(void)READ_BIT(RCC->AHB1ENR, BOARD_LCD_GPIO_CLOCK_ENABLE);

	gpio_write_high(BOARD_LCD_RD_GPIO, BOARD_LCD_RD_PIN);
	gpio_write_high(BOARD_LCD_WR_GPIO, BOARD_LCD_WR_PIN);
	gpio_write_high(BOARD_LCD_RS_GPIO, BOARD_LCD_RS_PIN);
	gpio_write_high(BOARD_LCD_CS_GPIO, BOARD_LCD_CS_PIN);
	gpio_write_high(BOARD_LCD_RST_GPIO, BOARD_LCD_RST_PIN);
	// deixa d6/xp e d7/ym em nivel alto antes de mudar o modo dos pinos.
	display_write_bus((1U << 6U) | (1U << 7U));

	gpio_configure_output(BOARD_LCD_RD_GPIO, BOARD_LCD_RD_PIN);
	gpio_configure_output(BOARD_LCD_CS_GPIO, BOARD_LCD_CS_PIN);
	gpio_configure_output(BOARD_LCD_WR_GPIO, BOARD_LCD_WR_PIN | BOARD_LCD_RS_PIN);
	gpio_configure_output(BOARD_LCD_RST_GPIO,
					  BOARD_LCD_RST_PIN | BOARD_LCD_DATA_F_MASK);
	gpio_configure_output(BOARD_LCD_DATA_GPIO_D, BOARD_LCD_DATA_D_MASK);
	gpio_configure_output(BOARD_LCD_DATA_GPIO_E, BOARD_LCD_DATA_E_MASK);

	SET_BIT(CoreDebug->DEMCR, CoreDebug_DEMCR_TRCENA_Msk);
	DWT->CYCCNT = 0U;
	SET_BIT(DWT->CTRL, DWT_CTRL_CYCCNTENA_Msk);

	display_delay_ms(50U);
	gpio_write_low(BOARD_LCD_RST_GPIO, BOARD_LCD_RST_PIN);
	display_delay_ms(100U);
	gpio_write_high(BOARD_LCD_RST_GPIO, BOARD_LCD_RST_PIN);
	display_delay_ms(100U);

	for (command_index = 0U;
		 command_index < (sizeof(init_sequence) / sizeof(init_sequence[0]));
		 command_index++)
	{
		display_write_register(init_sequence[command_index].register_address,
							   init_sequence[command_index].value);
		if (init_sequence[command_index].delay_ms > 0U)
		{
			display_delay_ms(init_sequence[command_index].delay_ms);
		}
	}

	// rotacao 3 do mcufriend: 320x240 horizontal e girado em 180 graus.
	display_write_register(0x0060U, 0x2700U);
	display_write_register(0x0001U, 0x0100U);
	display_write_register(0x0003U, 0x1038U);
	display_write_register(0x0061U, 0x0001U);
	display_write_register(0x006AU, 0x0000U);
	display_fill_screen(DISPLAY_COLOR_BLACK);
}

// primitivas de desenho

void display_fill_screen(uint16_t color)
{
	display_fill_rect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, color);
}

void display_fill_rect(int16_t x, int16_t y, int16_t width, int16_t height,
					   uint16_t color)
{
	uint32_t pixel_count;

	if ((width <= 0) || (height <= 0) || (x >= (int16_t)DISPLAY_WIDTH) ||
		(y >= (int16_t)DISPLAY_HEIGHT))
	{
		return;
	}
	if (x < 0)
	{
		width = (int16_t)(width + x);
		x = 0;
	}
	if (y < 0)
	{
		height = (int16_t)(height + y);
		y = 0;
	}
	if ((int32_t)x + width > (int32_t)DISPLAY_WIDTH)
	{
		width = (int16_t)((int16_t)DISPLAY_WIDTH - x);
	}
	if ((int32_t)y + height > (int32_t)DISPLAY_HEIGHT)
	{
		height = (int16_t)((int16_t)DISPLAY_HEIGHT - y);
	}
	if ((width <= 0) || (height <= 0))
	{
		return;
	}

	display_begin_pixels((uint16_t)x, (uint16_t)y,
						 (uint16_t)width, (uint16_t)height);
	pixel_count = (uint32_t)(uint16_t)width * (uint32_t)(uint16_t)height;
	while (pixel_count > 0U)
	{
		display_write16(color);
		pixel_count--;
	}
	display_end_pixels();
}

void display_draw_pixel(int16_t x, int16_t y, uint16_t color)
{
	display_fill_rect(x, y, 1, 1, color);
}

void display_draw_fast_hline(int16_t x, int16_t y, int16_t width,
						 uint16_t color)
{
	display_fill_rect(x, y, width, 1, color);
}

void display_draw_fast_vline(int16_t x, int16_t y, int16_t height,
						 uint16_t color)
{
	display_fill_rect(x, y, 1, height, color);
}

void display_draw_rect(int16_t x, int16_t y, int16_t width, int16_t height,
					   uint16_t color)
{
	if ((width <= 0) || (height <= 0))
	{
		return;
	}
	display_draw_fast_hline(x, y, width, color);
	display_draw_fast_hline(x, (int16_t)(y + height - 1), width, color);
	display_draw_fast_vline(x, y, height, color);
	display_draw_fast_vline((int16_t)(x + width - 1), y, height, color);
}

void display_draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
					   uint16_t color)
{
	int16_t dx;
	int16_t dy;
	int16_t sx;
	int16_t sy;
	int16_t error;

	if (y0 == y1)
	{
		if (x1 < x0)
		{
			const int16_t swap = x0;
			x0 = x1;
			x1 = swap;
		}
		display_draw_fast_hline(x0, y0, (int16_t)(x1 - x0 + 1), color);
		return;
	}
	if (x0 == x1)
	{
		if (y1 < y0)
		{
			const int16_t swap = y0;
			y0 = y1;
			y1 = swap;
		}
		display_draw_fast_vline(x0, y0, (int16_t)(y1 - y0 + 1), color);
		return;
	}

	dx = display_abs((int16_t)(x1 - x0));
	dy = (int16_t)-display_abs((int16_t)(y1 - y0));
	sx = x0 < x1 ? 1 : -1;
	sy = y0 < y1 ? 1 : -1;
	error = (int16_t)(dx + dy);

	for (;;)
	{
		const int16_t error_twice = (int16_t)(2 * error);
		display_draw_pixel(x0, y0, color);
		if ((x0 == x1) && (y0 == y1))
		{
			break;
		}
		if (error_twice >= dy)
		{
			error = (int16_t)(error + dy);
			x0 = (int16_t)(x0 + sx);
		}
		if (error_twice <= dx)
		{
			error = (int16_t)(error + dx);
			y0 = (int16_t)(y0 + sy);
		}
	}
}

void display_draw_text(int16_t x, int16_t y, const char *text,
					   uint16_t foreground, uint16_t background, uint8_t size)
{
	const int16_t origin_x = x;

	if (size == 0U)
	{
		return;
	}
	while (*text != '\0')
	{
		if (*text == '\n')
		{
			x = origin_x;
			y = (int16_t)(y + (8 * size));
		}
		else
		{
			display_draw_char(x, y, *text, foreground, background, size);
			x = (int16_t)(x + (6 * size));
		}
		text++;
	}
}

// compartilhamento dos pinos com o touch

void display_prepare_touch(void)
{
	gpio_write_high(BOARD_LCD_CS_GPIO, BOARD_LCD_CS_PIN);
	gpio_write_high(BOARD_LCD_RD_GPIO, BOARD_LCD_RD_PIN);
	gpio_write_high(BOARD_LCD_WR_GPIO, BOARD_LCD_WR_PIN);
	__DSB();
}

void display_restore_after_touch(void)
{
	// prepara niveis iguais antes de reativar as saidas para evitar pulsos.
	display_set_touch_idle_levels();

	gpio_configure_output(BOARD_LCD_WR_GPIO, BOARD_LCD_WR_PIN | BOARD_LCD_RS_PIN);
	gpio_configure_output(BOARD_TOUCH_XP_GPIO, BOARD_TOUCH_XP_PIN);
	gpio_configure_output(BOARD_TOUCH_YM_GPIO, BOARD_TOUCH_YM_PIN);
	gpio_write_high(BOARD_LCD_CS_GPIO, BOARD_LCD_CS_PIN);
	gpio_write_high(BOARD_LCD_RD_GPIO, BOARD_LCD_RD_PIN);
	__DSB();
}
