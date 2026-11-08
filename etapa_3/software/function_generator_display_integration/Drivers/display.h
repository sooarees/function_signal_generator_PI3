#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

#define DISPLAY_CONTROLLER_ID 0x6809U
#define DISPLAY_WIDTH         320U
#define DISPLAY_HEIGHT        240U

#define DISPLAY_COLOR_BLACK   0x0000U
#define DISPLAY_COLOR_WHITE   0xFFFFU

void display_init(void);
void display_fill_screen(uint16_t color);
void display_fill_rect(int16_t x, int16_t y, int16_t width, int16_t height,
					   uint16_t color);
void display_draw_pixel(int16_t x, int16_t y, uint16_t color);
void display_draw_fast_hline(int16_t x, int16_t y, int16_t width,
						 uint16_t color);
void display_draw_fast_vline(int16_t x, int16_t y, int16_t height,
						 uint16_t color);
void display_draw_rect(int16_t x, int16_t y, int16_t width, int16_t height,
					   uint16_t color);
void display_draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
					   uint16_t color);
void display_draw_text(int16_t x, int16_t y, const char *text,
					   uint16_t foreground, uint16_t background, uint8_t size);

void display_prepare_touch(void);
void display_restore_after_touch(void);

#endif
