#ifndef SCANNER_H
#define SCANNER_H

#include <stdint.h>

#define SCANNER_NUM_SLOTS  32
#define SCANNER_BAR_PX     10   /* screen pixels per slot (32*10 = 320) */
#define SCANNER_SETTLE_MIN 50   /* ms minimum settle time per slot       */

void    Scanner_Init(void);
void    Scanner_SetConfig(int leftAngle, int rightAngle, int sweepTimeMs);
void    Scanner_Start(void);
void    Scanner_Stop(void);
void    Scanner_Task(void);
int32_t Scanner_GetSlotDist_mm(int slot);
int     Scanner_GetNearestSlot(void);
int     Scanner_IsRunning(void);
int     Scanner_SweepDone(void);
void    Scanner_ClearSweepDone(void);

#endif
