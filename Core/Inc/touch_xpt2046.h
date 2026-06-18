#ifndef TOUCH_XPT2046_H
#define TOUCH_XPT2046_H

#include <stdint.h>

#define TOUCH_X_MIN 350
#define TOUCH_X_MAX 3900
#define TOUCH_Y_MIN 250
#define TOUCH_Y_MAX 3900
#define TOUCH_SWAP_XY 0
#define TOUCH_INVERT_X 1
#define TOUCH_INVERT_Y 0

void    Touch_Init(void);
uint8_t Touch_IsPressed(void);
uint8_t Touch_GetXY(uint16_t *sx, uint16_t *sy);

#endif
