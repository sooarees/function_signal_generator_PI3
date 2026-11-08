#include "ui.h"

#include "display.h"
#include "signal_control.h"

// cores da interface

#define UI_BLACK   0x0000U
#define UI_WHITE   0xFFFFU
#define UI_GREY    0x4228U
#define UI_LTGREY  0x867FU
#define UI_DKBLUE  0x008CU
#define UI_BLUE    0x025FU
#define UI_CYAN    0x07FFU
#define UI_YELLOW  0x5FFFU

#define WAVE_SINE      SIGNAL_WAVEFORM_SINE
#define WAVE_SQUARE    SIGNAL_WAVEFORM_SQUARE
#define WAVE_TRIANGLE  SIGNAL_WAVEFORM_TRIANGLE
#define WAVE_SAW       SIGNAL_WAVEFORM_SAW
#define WAVE_COUNT     4U

// tipos usados pela interface

typedef enum
{
	PARAM_FREQ = 0,
	PARAM_AMP,
	PARAM_OFFSET,
	PARAM_DUTY,
	PARAM_COUNT
} parameter_t;

typedef enum
{
	SCREEN_HOME = 0,
	SCREEN_ADJUST
} screen_t;

typedef struct
{
	int16_t x;
	int16_t y;
	int16_t width;
	int16_t height;
} button_t;

// limites exibidos na tela

#define UI_FREQUENCY_MIN_HZ          1
#define UI_FREQUENCY_MAX_HZ          20000
#define UI_AMPLITUDE_MIN_TENTHS_VPP  0
#define UI_AMPLITUDE_MAX_TENTHS_VPP  98
#define UI_OFFSET_MIN_TENTHS_V       -50
#define UI_OFFSET_MAX_TENTHS_V       50
#define UI_DUTY_MIN_PERCENT          0
#define UI_DUTY_MAX_PERCENT          100

// estado atual da interface

static signal_waveform_t waveform;
static screen_t current_screen;
static parameter_t active_parameter;
static int32_t draft_adjust_value;
static uint16_t frequency_hz;
static uint8_t amplitude_tenth_vpp;
static int8_t offset_tenth_v;
static uint8_t duty_percent;
static uint32_t last_touch_ms;

// dados usados nos desenhos

static const int8_t sine_quarter[33] = {
	0, 5, 10, 15, 20, 24, 29, 34, 38, 43, 47,
	51, 56, 60, 63, 67, 71, 74, 77, 80, 83,
	86, 88, 90, 92, 94, 96, 97, 98, 99, 100,
	100, 100
};

static const button_t wave_buttons[WAVE_COUNT] = {
	{8, 30, 72, 60}, {84, 30, 72, 60},
	{160, 30, 72, 60}, {236, 30, 76, 60}
};

static const button_t parameter_buttons[PARAM_COUNT] = {
	{8, 102, 148, 56}, {164, 102, 148, 56},
	{8, 174, 148, 56}, {164, 174, 148, 56}
};

static const button_t cancel_button = {16, 198, 132, 34};
static const button_t save_button = {172, 198, 132, 34};
static const button_t minus_button = {16, 120, 44, 44};
static const button_t plus_button = {260, 120, 44, 44};

#define SLIDER_X 74
#define SLIDER_Y 132
#define SLIDER_WIDTH 172
#define SLIDER_HEIGHT 20

// conversoes e limites

static int32_t ui_map(int32_t value, int32_t input_start, int32_t input_end,
					  int32_t output_start, int32_t output_end)
{
	return ((value - input_start) * (output_end - output_start) /
		(input_end - input_start)) + output_start;
}

static int32_t ui_clamp(int32_t value, int32_t minimum, int32_t maximum)
{
	if (value < minimum) return minimum;
	if (value > maximum) return maximum;
	return value;
}

static int32_t float_to_tenths(float value)
{
	return (int32_t)((value * 10.0f) + (value >= 0.0f ? 0.5f : -0.5f));
}

static bool sync_from_signal_control(void)
{
	signal_control_settings_t settings;

	if (!signal_control_get_settings(&settings) ||
		(settings.waveform < SIGNAL_WAVEFORM_SINE) ||
		(settings.waveform > SIGNAL_WAVEFORM_SAW))
	{
		return false;
	}

	waveform = settings.waveform;
	frequency_hz = (uint16_t)ui_clamp((int32_t)settings.frequency_hz,
		UI_FREQUENCY_MIN_HZ, UI_FREQUENCY_MAX_HZ);
	amplitude_tenth_vpp = (uint8_t)ui_clamp(
		float_to_tenths(settings.amplitude_vpp),
		UI_AMPLITUDE_MIN_TENTHS_VPP, UI_AMPLITUDE_MAX_TENTHS_VPP);
	offset_tenth_v = (int8_t)ui_clamp(
		float_to_tenths(settings.offset_v),
		UI_OFFSET_MIN_TENTHS_V, UI_OFFSET_MAX_TENTHS_V);
	duty_percent = (uint8_t)ui_clamp((int32_t)settings.duty_percent,
		UI_DUTY_MIN_PERCENT, UI_DUTY_MAX_PERCENT);
	return true;
}

// montagem dos textos

static uint8_t ui_string_length(const char *text)
{
	uint8_t length = 0U;
	while (text[length] != '\0') length++;
	return length;
}

static void text_append_char(char *buffer, uint8_t *position, char character)
{
	if (*position < 15U)
	{
		buffer[*position] = character;
		(*position)++;
		buffer[*position] = '\0';
	}
}

static void text_append(char *buffer, uint8_t *position, const char *text)
{
	while (*text != '\0')
	{
		text_append_char(buffer, position, *text);
		text++;
	}
}

static void text_append_u32(char *buffer, uint8_t *position, uint32_t value)
{
	char digits[10];
	uint8_t count = 0U;

	do
	{
		digits[count] = (char)('0' + (value % 10U));
		count++;
		value /= 10U;
	}
	while (value > 0U);

	while (count > 0U)
	{
		count--;
		text_append_char(buffer, position, digits[count]);
	}
}

static void text_append_tenths(char *buffer, uint8_t *position, int16_t value)
{
	if (value < 0)
	{
		text_append_char(buffer, position, '-');
		value = (int16_t)-value;
	}
	text_append_u32(buffer, position, (uint16_t)value / 10U);
	text_append_char(buffer, position, '.');
	text_append_char(buffer, position, (char)('0' + ((uint16_t)value % 10U)));
}

static int32_t get_parameter_value(parameter_t parameter);

static void format_parameter(parameter_t parameter, bool compact, char *buffer)
{
	uint8_t position = 0U;
	const int32_t value =
		((current_screen == SCREEN_ADJUST) &&
		 (active_parameter == parameter)) ?
			draft_adjust_value : get_parameter_value(parameter);
	buffer[0] = '\0';

	if (parameter == PARAM_FREQ)
	{
		if (compact && (value >= 1000))
		{
			text_append_u32(buffer, &position, (uint32_t)value / 1000U);
			if (((uint32_t)value % 1000U) != 0U)
			{
				text_append_char(buffer, &position, '.');
				text_append_u32(buffer, &position,
					((uint32_t)value % 1000U) / 100U);
			}
			text_append(buffer, &position, "kHz");
		}
		else
		{
			text_append_u32(buffer, &position, (uint32_t)value);
			text_append(buffer, &position, "Hz");
		}
	}
	else if (parameter == PARAM_AMP)
	{
		text_append_tenths(buffer, &position, (int16_t)value);
		text_append(buffer, &position, "Vpp");
	}
	else if (parameter == PARAM_OFFSET)
	{
		if (value > 0) text_append_char(buffer, &position, '+');
		text_append_tenths(buffer, &position, (int16_t)value);
		text_append_char(buffer, &position, 'V');
	}
	else
	{
		text_append_u32(buffer, &position, (uint32_t)value);
		text_append_char(buffer, &position, '%');
	}
}

static bool inside(button_t button, int16_t x, int16_t y)
{
	return (x >= button.x) && (x < (button.x + button.width)) &&
		(y >= button.y) && (y < (button.y + button.height));
}

static bool inside_touch(button_t button, int16_t x, int16_t y)
{
	const int16_t margin = 6;
	return (x >= (button.x - margin)) &&
		(x < (button.x + button.width + margin)) &&
		(y >= (button.y - margin)) &&
		(y < (button.y + button.height + margin));
}

// leitura e aplicacao dos parametros

static const char *parameter_name(parameter_t parameter)
{
	if (parameter == PARAM_AMP) return "Amplitude";
	if (parameter == PARAM_OFFSET) return "Offset";
	if (parameter == PARAM_DUTY) return "Duty Cycle";
	return "Frequencia";
}

static int32_t get_parameter_value(parameter_t parameter)
{
	if (parameter == PARAM_FREQ) return frequency_hz;
	if (parameter == PARAM_AMP) return amplitude_tenth_vpp;
	if (parameter == PARAM_OFFSET) return offset_tenth_v;
	return duty_percent;
}

static bool apply_parameter_value(parameter_t parameter, int32_t value)
{
	if (parameter == PARAM_FREQ)
	{
		return signal_control_set_frequency((uint32_t)ui_clamp(value,
			UI_FREQUENCY_MIN_HZ, UI_FREQUENCY_MAX_HZ));
	}
	if (parameter == PARAM_AMP)
	{
		const int32_t tenths = ui_clamp(value,
			UI_AMPLITUDE_MIN_TENTHS_VPP, UI_AMPLITUDE_MAX_TENTHS_VPP);

		return signal_control_set_amplitude((float)tenths / 10.0f);
	}
	if (parameter == PARAM_OFFSET)
	{
		const int32_t tenths = ui_clamp(value,
			UI_OFFSET_MIN_TENTHS_V, UI_OFFSET_MAX_TENTHS_V);

		// o signal_control guarda o valor; a saida de offset sera ligada depois.
		return signal_control_set_offset((float)tenths / 10.0f);
	}

	return signal_control_set_duty((uint32_t)ui_clamp(value,
		UI_DUTY_MIN_PERCENT, UI_DUTY_MAX_PERCENT));
}

// formas de onda usadas apenas na visualizacao

static int16_t sine_approx(uint8_t phase)
{
	const uint8_t quadrant = phase >> 6U;
	uint8_t local = phase & 0x3FU;
	uint8_t index;
	uint8_t fraction;
	int16_t value;

	if ((quadrant == 1U) || (quadrant == 3U)) local = (uint8_t)(63U - local);
	index = local >> 1U;
	fraction = local & 1U;
	value = (int16_t)(sine_quarter[index] +
		(((sine_quarter[index + 1U] - sine_quarter[index]) * fraction) / 2));
	return quadrant >= 2U ? (int16_t)-value : value;
}

static int16_t preview_value(signal_waveform_t wave, uint8_t phase)
{
	if (wave == WAVE_SQUARE) return phase < 128U ? 100 : -100;
	if (wave == WAVE_TRIANGLE)
	{
		if (phase < 128U)
		{
			return (int16_t)ui_map(phase, 0, 127, -100, 100);
		}
		return (int16_t)ui_map(phase, 128, 255, 100, -100);
	}
	if (wave == WAVE_SAW) return (int16_t)ui_map(phase, 0, 255, -100, 100);
	return sine_approx(phase);
}

// tela principal

static void draw_tron_grid(void)
{
	int16_t coordinate;

	for (coordinate = 36; coordinate < (int16_t)DISPLAY_HEIGHT; coordinate += 48)
	{
		display_draw_fast_hline(0, coordinate, DISPLAY_WIDTH, UI_DKBLUE);
	}
	for (coordinate = 28; coordinate < (int16_t)DISPLAY_WIDTH; coordinate += 58)
	{
		display_draw_fast_vline(coordinate, 24, DISPLAY_HEIGHT - 24, UI_DKBLUE);
	}
	display_draw_line(0, 239, 62, 178, UI_BLUE);
	display_draw_line(319, 239, 258, 178, UI_BLUE);
}

static void draw_header(const char *title)
{
	display_fill_rect(0, 0, DISPLAY_WIDTH, 24, UI_BLUE);
	display_draw_text(8, 5, title, UI_WHITE, UI_BLUE, 2U);
}

static void draw_mini_wave(button_t button, signal_waveform_t wave,
						   bool selected)
{
	const uint16_t border = selected ? UI_CYAN : UI_LTGREY;
	const uint16_t fill = selected ? UI_DKBLUE : UI_BLACK;
	const int16_t margin = 7;
	const int16_t left = (int16_t)(button.x + margin);
	const int16_t top = (int16_t)(button.y + margin);
	const int16_t width = (int16_t)(button.width - (2 * margin));
	const int16_t height = (int16_t)(button.height - (2 * margin));
	const int16_t middle = (int16_t)(top + (height / 2));
	const uint16_t points = (uint16_t)width * 2U;
	int16_t previous_x = left;
	int16_t previous_y = middle;
	uint16_t index;

	display_fill_rect(button.x, button.y, button.width, button.height, fill);
	display_draw_rect(button.x, button.y, button.width, button.height, border);

	for (index = 0U; index < points; index++)
	{
		const uint8_t phase = (uint8_t)((index * 255U) / (points - 1U));
		const int16_t value = preview_value(wave, phase);
		const int16_t x = (int16_t)(left +
			((index * (uint16_t)(width - 1)) / (points - 1U)));
		int16_t y = (int16_t)(middle - ui_map(value, -120, 120,
			-(height / 2 - 2), height / 2 - 2));

		y = (int16_t)ui_clamp(y, top + 2, top + height - 3);
		if (index > 0U)
		{
			display_draw_line(previous_x, previous_y, x, y, UI_YELLOW);
		}
		previous_x = x;
		previous_y = y;

		if ((wave == WAVE_SAW) && (index == (points - 1U)))
		{
			int16_t low_y = (int16_t)(middle - ui_map(preview_value(wave, 0U),
				-120, 120, -(height / 2 - 2), height / 2 - 2));
			low_y = (int16_t)ui_clamp(low_y, top + 2, top + height - 3);
			display_draw_line(x, y, x, low_y, UI_YELLOW);
		}
	}
}

static void draw_wave_buttons(void)
{
	signal_waveform_t wave;
	for (wave = WAVE_SINE; wave < WAVE_COUNT; wave++)
	{
		draw_mini_wave(wave_buttons[wave], wave, waveform == wave);
	}
}

static void draw_parameter_button(parameter_t parameter)
{
	const button_t button = parameter_buttons[parameter];
	const bool enabled = (parameter != PARAM_DUTY) || (waveform == WAVE_SQUARE);
	const uint16_t border = enabled ? UI_LTGREY : UI_GREY;
	const uint16_t text_color = enabled ? UI_WHITE : UI_GREY;
	char value[16];

	display_fill_rect(button.x, button.y, button.width, button.height, UI_BLACK);
	display_draw_rect(button.x, button.y, button.width, button.height, border);
	display_draw_text((int16_t)(button.x + 8), (int16_t)(button.y + 6),
		parameter_name(parameter), UI_LTGREY, UI_BLACK, 2U);
	format_parameter(parameter, true, value);
	display_draw_text((int16_t)(button.x + 8), (int16_t)(button.y + 33),
		value, text_color, UI_BLACK, 2U);
}

static void draw_home_screen(void)
{
	parameter_t parameter;

	(void)sync_from_signal_control();
	current_screen = SCREEN_HOME;
	display_fill_screen(UI_BLACK);
	draw_tron_grid();
	draw_header("Gerador de Funcoes");
	draw_wave_buttons();
	for (parameter = PARAM_FREQ; parameter < PARAM_COUNT; parameter++)
	{
		draw_parameter_button(parameter);
	}
}

// tela de ajuste

static int16_t value_to_slider_position(void)
{
	const int16_t half = SLIDER_WIDTH / 2;

	if (active_parameter == PARAM_FREQ)
	{
		if (draft_adjust_value <= 1000)
		{
			return (int16_t)ui_map(draft_adjust_value,
				UI_FREQUENCY_MIN_HZ, 1000, 0, half);
		}
		return (int16_t)ui_map(draft_adjust_value, 1000,
			 UI_FREQUENCY_MAX_HZ,
			 half, SLIDER_WIDTH);
	}
	if (active_parameter == PARAM_AMP)
	{
		return (int16_t)ui_map(draft_adjust_value,
			UI_AMPLITUDE_MIN_TENTHS_VPP, UI_AMPLITUDE_MAX_TENTHS_VPP,
			0, SLIDER_WIDTH);
	}
	if (active_parameter == PARAM_OFFSET)
	{
		return (int16_t)ui_map(draft_adjust_value,
			UI_OFFSET_MIN_TENTHS_V, UI_OFFSET_MAX_TENTHS_V,
			0, SLIDER_WIDTH);
	}
	return (int16_t)ui_map(draft_adjust_value,
		UI_DUTY_MIN_PERCENT, UI_DUTY_MAX_PERCENT, 0, SLIDER_WIDTH);
}

static void set_value_from_slider(int16_t touch_x)
{
	const int16_t position = (int16_t)ui_clamp(touch_x - SLIDER_X,
		0, SLIDER_WIDTH);
	const int16_t half = SLIDER_WIDTH / 2;

	if (active_parameter == PARAM_FREQ)
	{
		if (position <= half)
		{
			draft_adjust_value = ui_map(position, 0, half,
				UI_FREQUENCY_MIN_HZ, 1000);
		}
		else
		{
			const int32_t raw = ui_map(position, half, SLIDER_WIDTH,
				1000, UI_FREQUENCY_MAX_HZ);
			draft_adjust_value = ui_clamp(((raw + 50) / 100) * 100,
				1000, UI_FREQUENCY_MAX_HZ);
		}
	}
	else if (active_parameter == PARAM_AMP)
	{
		draft_adjust_value = ui_map(position, 0, SLIDER_WIDTH,
			UI_AMPLITUDE_MIN_TENTHS_VPP, UI_AMPLITUDE_MAX_TENTHS_VPP);
	}
	else if (active_parameter == PARAM_OFFSET)
	{
		draft_adjust_value = ui_map(position, 0, SLIDER_WIDTH,
			UI_OFFSET_MIN_TENTHS_V, UI_OFFSET_MAX_TENTHS_V);
	}
	else
	{
		const int32_t raw = ui_map(position, 0, SLIDER_WIDTH,
			UI_DUTY_MIN_PERCENT, UI_DUTY_MAX_PERCENT);
		draft_adjust_value = ui_clamp(((raw + 1) / 3) * 3,
			UI_DUTY_MIN_PERCENT, UI_DUTY_MAX_PERCENT);
	}
}

static void draw_slider(void)
{
	const int16_t position = value_to_slider_position();
	const int16_t knob_x = (int16_t)(SLIDER_X + position);

	display_fill_rect(SLIDER_X - 8, SLIDER_Y - 18,
		SLIDER_WIDTH + 16, 58, UI_BLACK);
	display_draw_rect(SLIDER_X, SLIDER_Y, SLIDER_WIDTH, SLIDER_HEIGHT, UI_LTGREY);
	display_fill_rect(SLIDER_X + 2, SLIDER_Y + 7,
		SLIDER_WIDTH - 4, 6, UI_GREY);
	display_fill_rect(SLIDER_X + 2, SLIDER_Y + 7, position, 6, UI_CYAN);
	display_fill_rect(knob_x - 5, SLIDER_Y - 8, 11,
		SLIDER_HEIGHT + 16, UI_YELLOW);
}

static void update_adjust_value(void)
{
	char value[16];
	int16_t text_width;
	int16_t x;

	format_parameter(active_parameter, false, value);
	text_width = (int16_t)(ui_string_length(value) * 18U);
	x = (int16_t)(((int16_t)DISPLAY_WIDTH - text_width) / 2);
	display_fill_rect(0, 62, DISPLAY_WIDTH, 48, UI_BLACK);
	display_draw_text(x, 78, value, UI_WHITE, UI_BLACK, 3U);
}

static void draw_adjust_button(button_t button, const char *label, uint16_t fill)
{
	display_fill_rect(button.x, button.y, button.width, button.height, fill);
	display_draw_rect(button.x, button.y, button.width, button.height, UI_LTGREY);
	display_draw_text((int16_t)(button.x + 12), (int16_t)(button.y + 9),
		label, UI_WHITE, fill, 2U);
}

static void draw_adjust_screen(parameter_t parameter)
{
	(void)sync_from_signal_control();
	active_parameter = parameter;
	current_screen = SCREEN_ADJUST;
	draft_adjust_value = get_parameter_value(parameter);

	display_fill_screen(UI_BLACK);
	draw_tron_grid();
	draw_header(parameter_name(parameter));
	update_adjust_value();
	draw_slider();

	display_fill_rect(minus_button.x, minus_button.y,
		minus_button.width, minus_button.height, UI_DKBLUE);
	display_draw_rect(minus_button.x, minus_button.y,
		minus_button.width, minus_button.height, UI_LTGREY);
	display_draw_text((int16_t)(minus_button.x + 14),
		(int16_t)(minus_button.y + 9), "-", UI_WHITE, UI_DKBLUE, 3U);

	display_fill_rect(plus_button.x, plus_button.y,
		plus_button.width, plus_button.height, UI_DKBLUE);
	display_draw_rect(plus_button.x, plus_button.y,
		plus_button.width, plus_button.height, UI_LTGREY);
	display_draw_text((int16_t)(plus_button.x + 13),
		(int16_t)(plus_button.y + 9), "+", UI_WHITE, UI_DKBLUE, 3U);

	draw_adjust_button(cancel_button, "Cancelar", UI_BLACK);
	draw_adjust_button(save_button, "Salvar", UI_BLUE);
}

static void nudge_active_parameter(int8_t direction)
{
	if (active_parameter == PARAM_FREQ)
	{
		const int16_t step = draft_adjust_value < 1000 ? 1 : 100;
		draft_adjust_value = ui_clamp(
			draft_adjust_value + ((int32_t)direction * step),
			UI_FREQUENCY_MIN_HZ, UI_FREQUENCY_MAX_HZ);
	}
	else if (active_parameter == PARAM_AMP)
	{
		draft_adjust_value = ui_clamp(draft_adjust_value + direction,
			UI_AMPLITUDE_MIN_TENTHS_VPP, UI_AMPLITUDE_MAX_TENTHS_VPP);
	}
	else if (active_parameter == PARAM_OFFSET)
	{
		draft_adjust_value = ui_clamp(draft_adjust_value + direction,
			UI_OFFSET_MIN_TENTHS_V, UI_OFFSET_MAX_TENTHS_V);
	}
	else
	{
		draft_adjust_value = ui_clamp(
			draft_adjust_value + ((int32_t)direction * 3),
			UI_DUTY_MIN_PERCENT, UI_DUTY_MAX_PERCENT);
	}
	update_adjust_value();
	draw_slider();
}

// tratamento do touch

static void handle_home_touch(int16_t x, int16_t y)
{
	uint8_t index;

	for (index = 0U; index < WAVE_COUNT; index++)
	{
		if (inside_touch(wave_buttons[index], x, y))
		{
			const signal_waveform_t requested_waveform =
				(signal_waveform_t)index;

			if (signal_control_set_waveform(requested_waveform))
			{
				waveform = requested_waveform;
			}
			else
			{
				(void)sync_from_signal_control();
			}
			draw_wave_buttons();
			draw_parameter_button(PARAM_DUTY);
			return;
		}
	}
	for (index = 0U; index < PARAM_COUNT; index++)
	{
		if (inside_touch(parameter_buttons[index], x, y))
		{
			const parameter_t parameter = (parameter_t)index;
			if ((parameter != PARAM_DUTY) || (waveform == WAVE_SQUARE))
			{
				draw_adjust_screen(parameter);
			}
			return;
		}
	}
}

static void handle_adjust_touch(int16_t x, int16_t y)
{
	const button_t slider_touch = {
		SLIDER_X - 10, SLIDER_Y - 24, SLIDER_WIDTH + 20, 68
	};

	if (inside_touch(cancel_button, x, y))
	{
		// o rascunho e descartado; o valor confirmado nunca foi alterado.
		draw_home_screen();
		return;
	}
	if (inside_touch(save_button, x, y))
	{
		(void)apply_parameter_value(active_parameter, draft_adjust_value);
		draw_home_screen();
		return;
	}
	if (inside_touch(minus_button, x, y))
	{
		nudge_active_parameter(-1);
		return;
	}
	if (inside_touch(plus_button, x, y))
	{
		nudge_active_parameter(1);
		return;
	}
	if (inside(slider_touch, x, y))
	{
		set_value_from_slider(x);
		update_adjust_value();
		draw_slider();
	}
}

// interface publica

bool ui_init(void)
{
	waveform = WAVE_SINE;
	current_screen = SCREEN_HOME;
	active_parameter = PARAM_FREQ;
	draft_adjust_value = 0;
	frequency_hz = 0U;
	amplitude_tenth_vpp = 0U;
	offset_tenth_v = 0;
	duty_percent = 0U;
	last_touch_ms = 0U;
	if (!sync_from_signal_control())
	{
		return false;
	}
	draw_home_screen();
	return true;
}

bool ui_process_touch(const touch_point_t *point, uint32_t now_ms)
{
	const uint32_t repeat_delay_ms = current_screen == SCREEN_ADJUST ? 45U : 110U;

	if ((now_ms - last_touch_ms) <= repeat_delay_ms)
	{
		return false;
	}
	last_touch_ms = now_ms;

	if (current_screen == SCREEN_ADJUST)
	{
		handle_adjust_touch(point->x, point->y);
	}
	else
	{
		handle_home_touch(point->x, point->y);
	}
	return true;
}
