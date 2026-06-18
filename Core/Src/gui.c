#include "gui.h"
#include "tft_ili9341.h"
#include "touch_xpt2046.h"
#include "scanner.h"
#include <stdio.h>
#include <string.h>

extern void UART_SendText(const char *text);
extern volatile uint32_t appTickMs;

#define BAR_AREA_H 200
#define STATUS_Y 200
#define STATUS_H 40

#define BTN_W 36
#define BTN_H 26
#define VAL_W 48
#define ROW1_Y 48
#define ROW2_Y 93
#define ROW3_Y 138
#define COL_LBL 8
#define COL_MINUS 116
#define COL_VAL 158
#define COL_PLUS 212
#define COL_UNIT 254

#define HIT_W 60
#define HIT_H 38
#define HIT_MINUS_X (COL_MINUS - 12)
#define HIT_PLUS_X (COL_PLUS - 12)
#define HIT_ROW1_Y (ROW1_Y - 6)
#define HIT_ROW2_Y (ROW2_Y - 6)
#define HIT_ROW3_Y (ROW3_Y - 6)

#define START_X 80
#define START_Y 185
#define START_W 160
#define START_H 40

#define BACK_X 0
#define BACK_Y STATUS_Y
#define BACK_W 70
#define BACK_H STATUS_H

#define DEBOUNCE_MS 300u

static GuiScreen screen = GUI_CONFIG;
static int needFullDraw = 1;
static int needValDraw = 0;
static uint32_t lastTouchMs = 0;

static int cfgLeftAngle = 30;
static int cfgRightAngle = 150;
static int cfgSweepSec = 4;

static void DrawButton(int16_t x, int16_t y, int16_t w, int16_t h, const char *lbl, uint16_t bg, uint16_t fg, uint8_t sz) {
    TFT_FillRect(x, y, w, h, bg);
    TFT_DrawRect(x, y, w, h, fg);
    int16_t tx = (int16_t)(x + (w - (int16_t)(strlen(lbl) * 6u * sz)) / 2);
    int16_t ty = (int16_t)(y + (h - 8 * sz) / 2);
    TFT_DrawString(tx, ty, lbl, fg, bg, sz);
}

static uint8_t TouchInRect(uint16_t tx, uint16_t ty, int16_t rx, int16_t ry, int16_t rw, int16_t rh) {
    return (tx >= (uint16_t)rx && tx < (uint16_t)(rx + rw) && ty >= (uint16_t)ry && ty < (uint16_t)(ry + rh)) ? 1u : 0u;
}

static void DrawConfigValues(void) {
    char buf[8];

    TFT_FillRect(COL_VAL, ROW1_Y - 2, VAL_W, BTN_H + 4, TFT_BLACK);
    TFT_FillRect(COL_VAL, ROW2_Y - 2, VAL_W, BTN_H + 4, TFT_BLACK);
    TFT_FillRect(COL_VAL, ROW3_Y - 2, VAL_W, BTN_H + 4, TFT_BLACK);

    snprintf(buf, sizeof(buf), "%3d", cfgLeftAngle);
    TFT_DrawString(COL_VAL + 2, ROW1_Y + 4, buf, TFT_YELLOW, TFT_BLACK, 2);

    snprintf(buf, sizeof(buf), "%3d", cfgRightAngle);
    TFT_DrawString(COL_VAL + 2, ROW2_Y + 4, buf, TFT_YELLOW, TFT_BLACK, 2);

    snprintf(buf, sizeof(buf), "%3d", cfgSweepSec);
    TFT_DrawString(COL_VAL + 2, ROW3_Y + 4, buf, TFT_YELLOW, TFT_BLACK, 2);
}

static void DrawConfigScreen(void) {
    TFT_FillScreen(TFT_BLACK);

    TFT_DrawString(88, 10, "SONAR CONFIG", TFT_WHITE, TFT_BLACK, 2);

    TFT_DrawString(COL_LBL, ROW1_Y + 4, "L ANG:", TFT_WHITE, TFT_BLACK, 2);
    TFT_DrawString(COL_LBL, ROW2_Y + 4, "R ANG:", TFT_WHITE, TFT_BLACK, 2);
    TFT_DrawString(COL_LBL, ROW3_Y + 4, "SWEEP:", TFT_WHITE, TFT_BLACK, 2);

    DrawButton(COL_MINUS, ROW1_Y, BTN_W, BTN_H, "-", TFT_DKGRAY, TFT_WHITE, 2);
    DrawButton(COL_PLUS, ROW1_Y, BTN_W, BTN_H, "+", TFT_DKGRAY, TFT_WHITE, 2);
    DrawButton(COL_MINUS, ROW2_Y, BTN_W, BTN_H, "-", TFT_DKGRAY, TFT_WHITE, 2);
    DrawButton(COL_PLUS, ROW2_Y, BTN_W, BTN_H, "+", TFT_DKGRAY, TFT_WHITE, 2);
    DrawButton(COL_MINUS, ROW3_Y, BTN_W, BTN_H, "-", TFT_DKGRAY, TFT_WHITE, 2);
    DrawButton(COL_PLUS, ROW3_Y, BTN_W, BTN_H, "+", TFT_DKGRAY, TFT_WHITE, 2);

    TFT_DrawString(COL_UNIT, ROW1_Y + 4, "deg", TFT_GRAY, TFT_BLACK, 2);
    TFT_DrawString(COL_UNIT, ROW2_Y + 4, "deg", TFT_GRAY, TFT_BLACK, 2);
    TFT_DrawString(COL_UNIT, ROW3_Y + 4, "s  ", TFT_GRAY, TFT_BLACK, 2);

    DrawButton(START_X, START_Y, START_W, START_H, "START", TFT_DKGREEN, TFT_WHITE, 2);

    DrawConfigValues();
}

static void ConfigScreen_Task(void) {
    uint16_t tx, ty;

    if (needFullDraw) { DrawConfigScreen(); needFullDraw = 0; return; }
    if (needValDraw) { DrawConfigValues(); needValDraw = 0; }

    if (!Touch_GetXY(&tx, &ty)) return;
    if (appTickMs - lastTouchMs < DEBOUNCE_MS) return;
    lastTouchMs = appTickMs;

    {
        char buf[48];
        snprintf(buf, sizeof(buf), "TOUCH x=%d y=%d\r\n", tx, ty);
        UART_SendText(buf);
    }

    if (TouchInRect(tx, ty, HIT_MINUS_X, HIT_ROW1_Y, HIT_W, HIT_H)) {
        cfgLeftAngle -= 5;
        if (cfgLeftAngle < 0) cfgLeftAngle = 0;
        needValDraw = 1;
        { char buf[32]; snprintf(buf, sizeof(buf), "L_ANG- -> %d\r\n", cfgLeftAngle); UART_SendText(buf); }
    } else if (TouchInRect(tx, ty, HIT_PLUS_X, HIT_ROW1_Y, HIT_W, HIT_H)) {
        cfgLeftAngle += 5;
        if (cfgLeftAngle > cfgRightAngle - 10) cfgLeftAngle = cfgRightAngle - 10;
        needValDraw = 1;
        { char buf[32]; snprintf(buf, sizeof(buf), "L_ANG+ -> %d\r\n", cfgLeftAngle); UART_SendText(buf); }
    } else if (TouchInRect(tx, ty, HIT_MINUS_X, HIT_ROW2_Y, HIT_W, HIT_H)) {
        cfgRightAngle -= 5;
        if (cfgRightAngle < cfgLeftAngle + 10) cfgRightAngle = cfgLeftAngle + 10;
        needValDraw = 1;
        { char buf[32]; snprintf(buf, sizeof(buf), "R_ANG- -> %d\r\n", cfgRightAngle); UART_SendText(buf); }
    } else if (TouchInRect(tx, ty, HIT_PLUS_X, HIT_ROW2_Y, HIT_W, HIT_H)) {
        cfgRightAngle += 5;
        if (cfgRightAngle > 180) cfgRightAngle = 180;
        needValDraw = 1;
        { char buf[32]; snprintf(buf, sizeof(buf), "R_ANG+ -> %d\r\n", cfgRightAngle); UART_SendText(buf); }
    } else if (TouchInRect(tx, ty, HIT_MINUS_X, HIT_ROW3_Y, HIT_W, HIT_H)) {
        cfgSweepSec--;
        if (cfgSweepSec < 1) cfgSweepSec = 1;
        needValDraw = 1;
        { char buf[32]; snprintf(buf, sizeof(buf), "SWEEP- -> %ds\r\n", cfgSweepSec); UART_SendText(buf); }
    } else if (TouchInRect(tx, ty, HIT_PLUS_X, HIT_ROW3_Y, HIT_W, HIT_H)) {
        cfgSweepSec++;
        if (cfgSweepSec > 10) cfgSweepSec = 10;
        needValDraw = 1;
        { char buf[32]; snprintf(buf, sizeof(buf), "SWEEP+ -> %ds\r\n", cfgSweepSec); UART_SendText(buf); }
    } else if (TouchInRect(tx, ty, START_X, START_Y, START_W, START_H)) {
        { char buf[64]; snprintf(buf, sizeof(buf), "START pressed: L=%d R=%d T=%ds\r\n",
              cfgLeftAngle, cfgRightAngle, cfgSweepSec); UART_SendText(buf); }
        Scanner_SetConfig(cfgLeftAngle, cfgRightAngle, cfgSweepSec * 1000);
        GUI_SetScreen(GUI_RUN);
        Scanner_Start();
    } else {
        UART_SendText("TOUCH: no button hit\r\n");
    }
}

static int16_t prevBarH[SCANNER_NUM_SLOTS];
static int prevNearest = -2;

static void DrawRunStatic(void) {
    int i;
    TFT_FillScreen(TFT_BLACK);
    for (i = 0; i < SCANNER_NUM_SLOTS; i++) prevBarH[i] = -1;
    prevNearest = -2;

    TFT_FillRect(0, STATUS_Y, TFT_W, STATUS_H, TFT_DKGRAY);
    DrawButton(BACK_X, BACK_Y, BACK_W, BACK_H, "BACK", TFT_DKGRAY, TFT_WHITE, 2);
    TFT_DrawString(80, STATUS_Y + 12, "NEAR:", TFT_WHITE, TFT_DKGRAY, 2);
}

static void DrawBars(void) {
    int nearest = Scanner_GetNearestSlot();
    int i;
    char buf[20];

    for (i = 0; i < SCANNER_NUM_SLOTS; i++) {
        int32_t d = Scanner_GetSlotDist_mm(i);
        int16_t barH = 0;
        uint16_t col;

        if (d > 0) {
            barH = (int16_t)((int32_t)(4000 - d) * BAR_AREA_H / 4000);
            if (barH < 0) barH = 0;
            if (barH > BAR_AREA_H) barH = BAR_AREA_H;
        }

        int isNearest = (i == nearest && d > 0);
        int wasNearest = (i == prevNearest);
        if (barH == prevBarH[i] && isNearest == wasNearest) continue;
        prevBarH[i] = barH;

        col = isNearest ? TFT_RED : TFT_GREEN;

        int16_t bx = (int16_t)(i * SCANNER_BAR_PX);
        TFT_FillRect(bx, 0, SCANNER_BAR_PX, BAR_AREA_H - barH, TFT_BLACK);
        if (barH > 0) {
            TFT_FillRect(bx, BAR_AREA_H - barH, SCANNER_BAR_PX, barH, col);
        }
    }

    TFT_FillRect(140, STATUS_Y + 4, 170, STATUS_H - 8, TFT_DKGRAY);
    if (nearest >= 0) {
        int32_t nd = Scanner_GetSlotDist_mm(nearest);
        snprintf(buf, sizeof(buf), "%4d mm", (int)nd);
    } else {
        snprintf(buf, sizeof(buf), "  --- mm");
    }
    TFT_DrawString(144, STATUS_Y + 12, buf, TFT_YELLOW, TFT_DKGRAY, 2);
    prevNearest = nearest;
}

static void RunScreen_Task(void) {
    uint16_t tx, ty;

    if (needFullDraw) { DrawRunStatic(); needFullDraw = 0; return; }

    if (Scanner_SweepDone()) {
        DrawBars();
        Scanner_ClearSweepDone();
    }

    if (!Touch_GetXY(&tx, &ty)) return;
    if (appTickMs - lastTouchMs < DEBOUNCE_MS) return;
    lastTouchMs = appTickMs;

    if (TouchInRect(tx, ty, BACK_X, BACK_Y, BACK_W, BACK_H)) {
        UART_SendText("BACK pressed -> config screen\r\n");
        Scanner_Stop();
        GUI_SetScreen(GUI_CONFIG);
    }
}

void GUI_Init(void) {
    screen = GUI_CONFIG;
    needFullDraw = 1;
}

void GUI_SetScreen(GuiScreen s) {
    screen = s;
    needFullDraw = 1;
}

void GUI_Task(void) {
    switch (screen) {
    case GUI_CONFIG: ConfigScreen_Task(); break;
    case GUI_RUN:    RunScreen_Task();    break;
    }
}
