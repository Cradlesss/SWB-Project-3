#include "stepper.h"
#include "main.h"
#include "scanner.h"

extern volatile uint32_t appTickMs;

static const uint8_t seq[8][4] = {
    {1,0,0,0}, {1,1,0,0}, {0,1,0,0}, {0,1,1,0},
    {0,0,1,0}, {0,0,1,1}, {0,0,0,1}, {1,0,0,1}
};

static int32_t currentStep = 0;
static int32_t targetStep = 0;
static uint32_t lastStepMs = 0;
static int calibSet = 0;
static int32_t calibLeftStep = 0;
static int32_t calibCenterStep = 0;
static int32_t calibRightStep = 0;

static void ApplyStep(int32_t step) {
    int phase = (int)(((step % 8) + 8) % 8);
    HAL_GPIO_WritePin(STEP_IN1_GPIO_Port, STEP_IN1_Pin, seq[phase][0] ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(STEP_IN2_GPIO_Port, STEP_IN2_Pin, seq[phase][1] ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(STEP_IN3_GPIO_Port, STEP_IN3_Pin, seq[phase][2] ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(STEP_IN4_GPIO_Port, STEP_IN4_Pin, seq[phase][3] ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void Stepper_Init(void) {
    currentStep = 0;
    targetStep = 0;
    calibSet = 0;
    HAL_GPIO_WritePin(STEP_IN1_GPIO_Port, STEP_IN1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(STEP_IN2_GPIO_Port, STEP_IN2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(STEP_IN3_GPIO_Port, STEP_IN3_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(STEP_IN4_GPIO_Port, STEP_IN4_Pin, GPIO_PIN_RESET);
}

void Stepper_Task(void) {
    if (currentStep == targetStep) return;
    if (appTickMs - lastStepMs < STEPPER_STEP_DELAY_MS) return;
    lastStepMs = appTickMs;
    if (currentStep < targetStep) currentStep++;
    else currentStep--;
    ApplyStep(currentStep);
}

void Stepper_SetTarget(int32_t step) { targetStep = step; }
int32_t Stepper_GetCurrentStep(void) { return currentStep; }
int Stepper_IsBusy(void) { return currentStep != targetStep; }
void Stepper_JogStep(int delta) { targetStep += delta; }

void Stepper_CalibSetLeft(void) { calibLeftStep = currentStep; calibSet |= 0x1; }
void Stepper_CalibSetCenter(void) { calibCenterStep = currentStep; calibSet |= 0x2; }
void Stepper_CalibSetRight(void) { calibRightStep = currentStep; calibSet |= 0x4; }
int Stepper_IsCalibrated(void) { return calibSet == 0x7; }
int Stepper_GetCalibSet(void) { return calibSet; }

void Stepper_GoToSlot(int slot) {
    if (calibSet != 0x7 || slot < 0) return;
    if (slot >= SCANNER_NUM_SLOTS) slot = SCANNER_NUM_SLOTS - 1;
    int32_t step;
    int half = SCANNER_NUM_SLOTS / 2;
    if (slot <= half) {
        step = calibLeftStep + (calibCenterStep - calibLeftStep) * slot / half;
    } else {
        step = calibCenterStep + (calibRightStep - calibCenterStep) * (slot - half) / (SCANNER_NUM_SLOTS - 1 - half);
    }
    targetStep = step;
}
