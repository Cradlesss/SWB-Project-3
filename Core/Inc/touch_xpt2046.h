#ifndef TOUCH_XPT2046_H
#define TOUCH_XPT2046_H

#include <stdint.h>

/* Adjust these after testing with your panel */
#define TOUCH_X_MIN   200
#define TOUCH_X_MAX  3800
#define TOUCH_Y_MIN   200
#define TOUCH_Y_MAX  3800
#define TOUCH_SWAP_XY    1   /* 1 = swap raw X/Y for landscape, 0 = no swap */
#define TOUCH_INVERT_X   0   /* 1 = invert mapped X                          */
#define TOUCH_INVERT_Y   1   /* 1 = invert mapped Y                          */

void    Touch_Init(void);
uint8_t Touch_IsPressed(void);
uint8_t Touch_GetXY(uint16_t *sx, uint16_t *sy);

#endif
