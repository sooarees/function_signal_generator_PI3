#ifndef TOUCH_H
#define TOUCH_H

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
	int16_t x;
	int16_t y;
	uint16_t raw_x;
	uint16_t raw_y;
	uint16_t pressure;
} touch_point_t;

void touch_init(void);
bool touch_read(touch_point_t *point);

#endif
