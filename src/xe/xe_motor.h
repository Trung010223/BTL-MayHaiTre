#ifndef XE_MOTOR_H
#define XE_MOTOR_H

void initMotor();
void run(int in1, int in2, int enPin, int speed);
int rampTo(int cur, int target, int step);
void setMotorSpeed(int motorA, int motorB, int motorC, int motorD, int motorE);
void stopAllMotors();

extern int curA, curB, curC, curD, curE;
extern int tocDoCanBang, tocDoBanhLai, buocRamp;

#endif
