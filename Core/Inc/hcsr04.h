#ifndef HCSR04_H
#define HCSR04_H

#include <stdint.h>

void    HCSR04_Init(void);
void    HCSR04_Task(void);
void    HCSR04_EchoIRQ(void);
void    HCSR04_Trigger(void);
int32_t HCSR04_GetDistance_mm(void);
uint8_t HCSR04_IsReady(void);
void    HCSR04_ClearReady(void);

#endif
