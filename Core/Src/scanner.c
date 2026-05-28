#include "scanner.h"
#include "hcsr04.h"
#include "servo.h"

extern volatile uint32_t appTickMs;

typedef enum { SC_IDLE, SC_SETTLING, SC_MEASURING } ScState;

static ScState  scState    = SC_IDLE;
static int      curSlot    = 0;
static uint32_t settleTs   = 0;
static uint32_t settleMs   = SCANNER_SETTLE_MIN;
static int      running    = 0;
static int      sweepDone  = 0;

static int     leftAngle   = 30;
static int     rightAngle  = 150;
static int     sweepTimeMs = 4000;

static int32_t filtered[SCANNER_NUM_SLOTS];

static int SlotAngle(int slot) {
    if (SCANNER_NUM_SLOTS <= 1) return leftAngle;
    return leftAngle + slot * (rightAngle - leftAngle) / (SCANNER_NUM_SLOTS - 1);
}

static void BeginSlot(int slot) {
    Servo_SetAngle(SlotAngle(slot));
    settleTs = appTickMs;
    scState  = SC_SETTLING;
}

void Scanner_Init(void) {
    int i;
    for (i = 0; i < SCANNER_NUM_SLOTS; i++) filtered[i] = -1;
    scState  = SC_IDLE;
    running  = 0;
    sweepDone = 0;
}

void Scanner_SetConfig(int la, int ra, int stMs) {
    leftAngle   = la;
    rightAngle  = ra;
    sweepTimeMs = stMs;
    /* Divide total sweep time evenly; respect minimum settle */
    settleMs = (uint32_t)(stMs / SCANNER_NUM_SLOTS);
    if (settleMs < SCANNER_SETTLE_MIN) settleMs = SCANNER_SETTLE_MIN;
}

void Scanner_Start(void) {
    int i;
    for (i = 0; i < SCANNER_NUM_SLOTS; i++) filtered[i] = -1;
    curSlot  = 0;
    sweepDone = 0;
    running  = 1;
    BeginSlot(0);
}

void Scanner_Stop(void) {
    running  = 0;
    scState  = SC_IDLE;
    sweepDone = 0;
}

void Scanner_Task(void) {
    if (!running) return;

    switch (scState) {
    case SC_SETTLING:
        if (appTickMs - settleTs >= settleMs) {
            HCSR04_ClearReady();
            HCSR04_Trigger();
            scState = SC_MEASURING;
        }
        break;

    case SC_MEASURING:
        HCSR04_Task();
        if (HCSR04_IsReady()) {
            int32_t raw = HCSR04_GetDistance_mm();
            HCSR04_ClearReady();

            /* EMA filter: 75% old + 25% new (skip if invalid) */
            if (raw > 0) {
                if (filtered[curSlot] < 0)
                    filtered[curSlot] = raw;
                else
                    filtered[curSlot] = (filtered[curSlot] * 3 + raw) / 4;
            }

            curSlot++;
            if (curSlot >= SCANNER_NUM_SLOTS) {
                curSlot   = 0;
                sweepDone = 1;
            }
            BeginSlot(curSlot);
        }
        break;

    default:
        break;
    }
}

int32_t Scanner_GetSlotDist_mm(int slot) {
    if (slot < 0 || slot >= SCANNER_NUM_SLOTS) return -1;
    return filtered[slot];
}

int Scanner_GetNearestSlot(void) {
    int     best   = -1;
    int32_t bestD  = 0x7FFFFFFF;
    int     i;
    for (i = 0; i < SCANNER_NUM_SLOTS; i++) {
        if (filtered[i] > 0 && filtered[i] < bestD) {
            bestD = filtered[i];
            best  = i;
        }
    }
    return best;
}

int Scanner_IsRunning(void)  { return running;   }
int Scanner_SweepDone(void)  { return sweepDone; }
void Scanner_ClearSweepDone(void) { sweepDone = 0; }
