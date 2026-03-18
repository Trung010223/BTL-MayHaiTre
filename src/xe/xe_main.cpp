#include <Arduino.h>
#include "xe.h"
#include "xe_motor.h"
#include "xe_imu.h"
#include "xe_espnow.h"
#include "xe_failsafe.h"

void initXe() {
  initMotor();
  initIMU();
  initESPNow();
  initFailsafe();
  Serial.println("[XE] Init xong!");
}

void updateXe() {
  watchdogReset();
  
  updateIMU();
  checkVehicleFailsafe();
  xuLyCanBang(pitchEMA, rollEMA);
  sendTelemetry();
}
