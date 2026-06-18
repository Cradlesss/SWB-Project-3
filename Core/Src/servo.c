#include "servo.h"
#include "tim.h"

static int curAngle = 90;

void Servo_Init(void) {
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
    Servo_SetAngle(90);
}

void Servo_SetAngle(int angle) {
    if (angle < SERVO_ANGLE_MIN) angle = SERVO_ANGLE_MIN;
    if (angle > SERVO_ANGLE_MAX) angle = SERVO_ANGLE_MAX;
    uint32_t pulse = 1000u + (uint32_t)angle * 1000u / 180u;
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, pulse);
    curAngle = angle;
}

int Servo_GetAngle(void) { return curAngle; }
