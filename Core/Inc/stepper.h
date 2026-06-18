#ifndef STEPPER_H
#define STEPPER_H

#include <stdint.h>

#define STEPPER_STEP_DELAY_MS 2
#define STEPPER_JOG_STEPS 20

void Stepper_Init(void);
void Stepper_Task(void);
void Stepper_SetTarget(int32_t step);
int32_t Stepper_GetCurrentStep(void);
int Stepper_IsBusy(void);
void Stepper_JogStep(int delta);
void Stepper_CalibSetLeft(void);
void Stepper_CalibSetCenter(void);
void Stepper_CalibSetRight(void);
int Stepper_IsCalibrated(void);
int Stepper_GetCalibSet(void);
void Stepper_GoToSlot(int slot);

#endif
