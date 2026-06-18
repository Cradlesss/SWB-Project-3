#ifndef HCSR04_H
#define HCSR04_H

#include <stdint.h>
#include "stm32g4xx_hal.h"

void HCSR04_Init(void);
void HCSR04_Trigger(void);
void HCSR04_Task(void);
void HCSR04_IC_Callback(TIM_HandleTypeDef *htim);
int32_t HCSR04_GetDistance_mm(void);
uint8_t HCSR04_IsReady(void);
void HCSR04_ClearReady(void);

#endif
