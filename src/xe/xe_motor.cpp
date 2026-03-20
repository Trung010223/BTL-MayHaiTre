#include <Arduino.h>
#include "xe_motor.h"

// Motor pinout
const int ENA = 32; const int IN1 = 33; const int IN2 = 25;
const int ENB = 26; const int IN3 = 27; const int IN4 = 14;
const int ENC = 12; const int IN5 = 13; const int IN6 = 15;
const int END = 2;  const int IN7 = 4;  const int IN8 = 5;
const int ENE = 16; const int IN9 = 18; const int IN10 = 19;

const int freq = 5000, res = 8;

int curA = 0, curB = 0, curC = 0, curD = 0, curE = 0;
int tocDoCanBang = 80, tocDoBanhLai = 70, buocRamp = 8;

void initMotor() {
  // Setup PWM channels for each motor
  ledcSetup(0, freq, res); ledcAttachPin(ENA, 0);
  ledcSetup(1, freq, res); ledcAttachPin(ENB, 1);
  ledcSetup(2, freq, res); ledcAttachPin(ENC, 2);
  ledcSetup(3, freq, res); ledcAttachPin(END, 3);
  ledcSetup(4, freq, res); ledcAttachPin(ENE, 4);

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(IN5, OUTPUT); pinMode(IN6, OUTPUT);
  pinMode(IN7, OUTPUT); pinMode(IN8, OUTPUT);
  pinMode(IN9, OUTPUT); pinMode(IN10, OUTPUT);

  Serial.println("[MOTOR] Init xong");
}

void run(int in1, int in2, int enPin, int speed) {
  // Map EN pins to their channel numbers
  int channel;
  if (enPin == ENA) channel = 0;
  else if (enPin == ENB) channel = 1;
  else if (enPin == ENC) channel = 2;
  else if (enPin == END) channel = 3;
  else channel = 4; // ENE

  if (speed > 0) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    ledcWrite(channel, speed);
  } else if (speed < 0) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    ledcWrite(channel, -speed);
  } else {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    ledcWrite(channel, 0);
  }
}

int rampTo(int cur, int target, int step) {
  if (cur < target) return min(cur + step, target);
  if (cur > target) return max(cur - step, target);
  return cur;
}

void setMotorSpeed(int motorA, int motorB, int motorC, int motorD, int motorE) {
  curA = rampTo(curA, motorA, buocRamp);
  curB = rampTo(curB, motorB, buocRamp);
  curC = rampTo(curC, motorC, buocRamp);
  curD = rampTo(curD, motorD, buocRamp);
  curE = rampTo(curE, motorE, buocRamp);

  run(IN1, IN2, ENA, curA); run(IN3, IN4, ENB, curB);
  run(IN5, IN6, ENC, curC); run(IN7, IN8, END, curD);
  run(IN9, IN10, ENE, curE);
}

void stopAllMotors() {
  run(IN1, IN2, ENA, 0); run(IN3, IN4, ENB, 0);
  run(IN5, IN6, ENC, 0); run(IN7, IN8, END, 0);
  run(IN9, IN10, ENE, 0);
  curA = curB = curC = curD = curE = 0;
}
