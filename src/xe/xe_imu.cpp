#include <Arduino.h>
#include <Wire.h>
#include <MPU6050_light.h>
#include "xe_imu.h"
#include "xe_motor.h"

MPU6050 mpu6050(Wire);

const float EMA_ALPHA = 0.15f;
float pitchEMA = 0.0f, rollEMA = 0.0f;
bool emaInitialized = false;

const float HYST_HIGH = 4.0f;
const float HYST_LOW = 1.5f;
bool pitchActive = false, rollActive = false;

float zeroPitch = 0.0, zeroRoll = 0.0;
bool cheDoCanBang = true;

extern bool latestCmd_stop;
extern int latestCmd_direction;

void initIMU() {
  Wire.begin();
  mpu6050.begin();
  mpu6050.calcOffsets();
  Serial.println("[IMU] Calibrate xong");
}

void updateEMA(float rawPitch, float rawRoll) {
  if (!emaInitialized) {
    pitchEMA = rawPitch;
    rollEMA = rawRoll;
    emaInitialized = true;
    return;
  }
  pitchEMA = EMA_ALPHA * rawPitch + (1.0f - EMA_ALPHA) * pitchEMA;
  rollEMA = EMA_ALPHA * rawRoll + (1.0f - EMA_ALPHA) * rollEMA;
}

bool updateHysteresis(float val, bool &active) {
  float absVal = fabs(val);
  if (absVal > HYST_HIGH) active = true;
  else if (absVal < HYST_LOW) active = false;
  return active;
}

String angleToLabel(float pitch, float roll) {
  float m = max(fabs(pitch), fabs(roll));
  if (m < HYST_LOW) return "BALANCED";
  if (m < HYST_HIGH) return "NEAR_BALANCE";
  if (m < 15.0f) return "TILTED";
  if (m < 30.0f) return "HEAVY_TILT";
  return "FALLEN";
}

void updateIMU() {
  mpu6050.update();
  float rawPitch = mpu6050.getAngleX() - zeroPitch;
  float rawRoll = mpu6050.getAngleY() - zeroRoll;
  updateEMA(rawPitch, rawRoll);
}

void xuLyCanBang(float pitch, float roll) {
  if (!cheDoCanBang || latestCmd_stop) {
    stopAllMotors();
    pitchActive = rollActive = false;
    return;
  }

  bool doPitch = updateHysteresis(pitch, pitchActive);
  bool doRoll = updateHysteresis(roll, rollActive);

  int pVal = 0, rVal = 0;
  if (doPitch) pVal = (pitch > 0) ? tocDoCanBang : -tocDoCanBang;
  if (doRoll) rVal = (roll > 0) ? tocDoCanBang : -tocDoCanBang;

  int tA = constrain(pVal - rVal, -tocDoCanBang, tocDoCanBang);
  int tB = constrain(pVal + rVal, -tocDoCanBang, tocDoCanBang);
  int tC = constrain(-pVal - rVal, -tocDoCanBang, tocDoCanBang);
  int tD = constrain(-pVal + rVal, -tocDoCanBang, tocDoCanBang);

  int tE = 0;
  if (latestCmd_direction == 1) tE = tocDoBanhLai;
  if (latestCmd_direction == -1) tE = -tocDoBanhLai;

  setMotorSpeed(tA, tB, tC, tD, tE);
}
