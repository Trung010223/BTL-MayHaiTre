#pragma once

#include <Adafruit_PWMServoDriver.h>
#include <PubSubClient.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <WiFi.h>

struct TelemetryPacket {
  float pitch;
  float roll;
  int curA;
  int curB;
  int curC;
  int curD;
  int curE;
  bool isBalanced;
};

struct CmdPacket {
  int speed;
  int direction;
  bool stop;
};

extern WebServer server;
extern WebSocketsServer webSocket;
extern WiFiClient espClient;
extern PubSubClient mqtt;
extern uint8_t nodeMAC[6];

extern TelemetryPacket latestTele;
extern bool newTeleAvailable;

extern Adafruit_PWMServoDriver pwm;
extern int pos[4];
extern bool autoMode;
extern bool resetAuto;
extern bool vacuumRunning;

void sendCmdToVehicle(int speed, int direction, bool stop);
