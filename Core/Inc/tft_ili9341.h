#ifndef TFT_ILI9341_H
#define TFT_ILI9341_H

#include <stdint.h>

#define TFT_W  320
#define TFT_H  240

#define TFT_BLACK   0x0000u
#define TFT_WHITE   0xFFFFu
#define TFT_RED     0xF800u
#define TFT_GREEN   0x07E0u
#define TFT_BLUE    0x001Fu
#define TFT_YELLOW  0xFFE0u
#define TFT_CYAN    0x07FFu
#define TFT_GRAY    0x7BEFu
#define TFT_ORANGE  0xFD20u
#define TFT_DKGREEN 0x03E0u
#define TFT_DKGRAY  0x39E7u

void TFT_Init(void);
void TFT_FillScreen(uint16_t color);
void TFT_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void TFT_DrawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void TFT_DrawChar(int16_t x, int16_t y, char c, uint16_t fg, uint16_t bg, uint8_t sz);
void TFT_DrawString(int16_t x, int16_t y, const char *s, uint16_t fg, uint16_t bg, uint8_t sz);

#endif
