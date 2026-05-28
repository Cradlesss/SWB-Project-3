#include "servo.h"
#include "tim.h"

/* Servo on TIM2_CH3 / PB10 (Arduino D6).
   TIM2 period=19999 @ 1 MHz -> 50 Hz.
   TIM2 is shared with HCSR04 us-counter; HAL_TIM_Base_Start
   in HCSR04_Init starts the counter, HAL_TIM_PWM_Start here
   enables the CH3 output on the already-running timer. */

static int curAngle = 90;

void Servo_Init(void) {
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
    Servo_SetAngle(90);
}

void Servo_SetAngle(int angle) {
    if (angle < SERVO_ANGLE_MIN) angle = SERVO_ANGLE_MIN;
    if (angle > SERVO_ANGLE_MAX) angle = SERVO_ANGLE_MAX;
    /* 0 deg -> 1000 us, 180 deg -> 2000 us, linear */
    uint32_t pulse = 1000u + (uint32_t)angle * 1000u / 180u;
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, pulse);
    curAngle = angle;
}

int Servo_GetAngle(void) { return curAngle; }
