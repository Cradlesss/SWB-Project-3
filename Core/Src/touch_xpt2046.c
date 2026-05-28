#include "touch_xpt2046.h"
#include "tft_ili9341.h"
#include "main.h"
#include "spi.h"

/* XPT2046 channel commands (12-bit differential, power-down between) */
#define CMD_X   0xD0u
#define CMD_Y   0x90u
#define CMD_Z1  0xB0u

static uint16_t ReadChannel(uint8_t cmd) {
    uint8_t tx[3] = {cmd, 0x00u, 0x00u};
    uint8_t rx[3] = {0, 0, 0};
    HAL_GPIO_WritePin(TOUCH_CS_GPIO_Port, TOUCH_CS_Pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(&hspi1, tx, rx, 3, 100);
    HAL_GPIO_WritePin(TOUCH_CS_GPIO_Port, TOUCH_CS_Pin, GPIO_PIN_SET);
    return (uint16_t)(((rx[1] << 8) | rx[2]) >> 3);
}

void Touch_Init(void) {
    HAL_GPIO_WritePin(TOUCH_CS_GPIO_Port, TOUCH_CS_Pin, GPIO_PIN_SET);
}

uint8_t Touch_IsPressed(void) {
    uint16_t z1 = ReadChannel(CMD_Z1);
    return (z1 > 100u) ? 1u : 0u;
}

uint8_t Touch_GetXY(uint16_t *sx, uint16_t *sy) {
    uint16_t rawX, rawY;
    int32_t  mx, my;

    if (!Touch_IsPressed()) return 0;

    rawX = ReadChannel(CMD_X);
    rawY = ReadChannel(CMD_Y);

    /* Average two reads for stability */
    rawX = (uint16_t)((rawX + ReadChannel(CMD_X)) / 2u);
    rawY = (uint16_t)((rawY + ReadChannel(CMD_Y)) / 2u);

#if TOUCH_SWAP_XY
    /* Landscape: raw Y maps to screen X, raw X maps to screen Y */
    mx = ((int32_t)rawY - TOUCH_Y_MIN) * TFT_W / (TOUCH_X_MAX - TOUCH_X_MIN);
    my = ((int32_t)rawX - TOUCH_X_MIN) * TFT_H / (TOUCH_Y_MAX - TOUCH_Y_MIN);
#else
    mx = ((int32_t)rawX - TOUCH_X_MIN) * TFT_W / (TOUCH_X_MAX - TOUCH_X_MIN);
    my = ((int32_t)rawY - TOUCH_Y_MIN) * TFT_H / (TOUCH_Y_MAX - TOUCH_Y_MIN);
#endif

#if TOUCH_INVERT_X
    mx = TFT_W - 1 - mx;
#endif
#if TOUCH_INVERT_Y
    my = TFT_H - 1 - my;
#endif

    if (mx < 0) mx = 0;
    if (mx >= TFT_W) mx = TFT_W - 1;
    if (my < 0) my = 0;
    if (my >= TFT_H) my = TFT_H - 1;

    *sx = (uint16_t)mx;
    *sy = (uint16_t)my;
    return 1;
}
