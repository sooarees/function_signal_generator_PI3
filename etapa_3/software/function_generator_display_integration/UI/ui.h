#ifndef UI_H
#define UI_H

#include <stdbool.h>
#include <stdint.h>

#include "touch.h"

bool ui_init(void);
bool ui_process_touch(const touch_point_t *point, uint32_t now_ms);

#endif
