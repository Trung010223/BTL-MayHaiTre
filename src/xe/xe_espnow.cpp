#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include "xe_espnow.h"

uint8_t gatewayMAC[] = {0xF0, 0x24, 0xF9, 0x45, 0xBE, 0xD4};

typedef struct {
  float pitch, roll;
  int curA, curB, curC, curD, curE;
  bool isBalanced;
} TelemetryPacket;

typedef struct {
  int speed, direction;
  bool stop;
} CmdPacket;

int latestCmd_speed = 0;
int latestCmd_direction = 0;
bool latestCmd_stop = true;
unsigned long lastCmdTime = 0;
unsigned long lastTelemetry = 0;

void OnDataRecv(const uint8_t *mac, const uint8_t *data, int len) {
  (void)mac;
  (void)len;
  CmdPacket cmd;
  memcpy(&cmd, data, sizeof(cmd));
  
  latestCmd_speed = cmd.speed;
  latestCmd_direction = cmd.direction;
  latestCmd_stop = cmd.stop;
  lastCmdTime = millis();
  
  Serial.println("[ESP-NOW] Nhan lenh tu Gateway");
}

void initESPNow() {
  WiFi.mode(WIFI_STA);
  esp_now_init();
  esp_now_register_recv_cb(OnDataRecv);

  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, gatewayMAC, 6);
  peer.channel = 0;
  peer.encrypt = false;
  esp_now_add_peer(&peer);

  Serial.println("[ESP-NOW] Node san sang");
}

void sendTelemetry() {
  if (millis() - lastTelemetry > 500) {
    TelemetryPacket tele;
    tele.pitch = pitchEMA;
    tele.roll = rollEMA;
    tele.curA = curA; tele.curB = curB;
    tele.curC = curC; tele.curD = curD;
    tele.curE = curE;
    tele.isBalanced = (!pitchActive && !rollActive);

    esp_now_send(gatewayMAC, (uint8_t *)&tele, sizeof(tele));
    lastTelemetry = millis();
  }
}
