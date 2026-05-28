#ifndef TOUCH_XPT2046_H
#define TOUCH_XPT2046_H

#include <stdint.h>

/* Calibrated for this panel (derived from measured raw ADC values) */
#define TOUCH_X_MIN   350   /* raw X at screen right edge (X=319) */
#define TOUCH_X_MAX  3900   /* raw X at screen left  edge (X=0)   */
#define TOUCH_Y_MIN   250   /* raw Y at screen top   edge (Y=0)   */
#define TOUCH_Y_MAX  3900   /* raw Y at screen bottom edge (Y=239) */
#define TOUCH_SWAP_XY    0   /* raw X → screen X, raw Y → screen Y */
#define TOUCH_INVERT_X   1   /* raw X decreases as screen X increases */
#define TOUCH_INVERT_Y   0   /* raw Y increases as screen Y increases */

void    Touch_Init(void);
uint8_t Touch_IsPressed(void);
uint8_t Touch_GetXY(uint16_t *sx, uint16_t *sy);

#endif
