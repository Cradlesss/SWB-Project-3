#ifndef SERVO_H
#define SERVO_H

#define SERVO_ANGLE_MIN   0
#define SERVO_ANGLE_MAX   180

void Servo_Init(void);
void Servo_SetAngle(int angle);
int  Servo_GetAngle(void);

#endif
