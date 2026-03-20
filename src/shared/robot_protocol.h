#pragma once

typedef struct TelemetryPacket {
  float pitch;
  float roll;
  int curA;
  int curB;
  int curC;
  int curD;
  int curE;
  bool isBalanced;
} TelemetryPacket;

typedef struct CmdPacket {
  int speed;
  int direction;
  int lift;
  bool stop;
} CmdPacket;