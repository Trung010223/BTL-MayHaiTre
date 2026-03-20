#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <PubSubClient.h>
#include <esp_now.h>

#include "../app_context.h"
#include "../network/config_portal.h"
#include "../dongco/control_module.h"
#include "../quat_hut/vacuum_module.h"
#include "../network/mqtt_fsm.h"
#include "../network/store_forward.h"
#include "../web/dashboard_css.h"
#include "../web/dashboard_html.h"
#include "../web/dashboard_js.h"
#include "../web/websocket_module.h"
#include "canh_tay.h"

#define I2C_SDA 21
#define I2C_SCL 22

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);
WiFiClient espClient;
PubSubClient mqtt(espClient);
uint8_t nodeMAC[] = {0x30, 0xC6, 0xF7, 0x30, 0x79, 0xCC};

TelemetryPacket latestTele = {0, 0, 0, 0, 0, 0, 0, false};
bool newTeleAvailable = false;

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40, Wire);
int pos[4] = {330, 150, 300, 410};
bool autoMode = false;
bool resetAuto = false;
bool vacuumRunning = false;

void OnDataRecv(const uint8_t *mac, const uint8_t *data, int len) {
  (void)mac;
  memcpy(&latestTele, data, sizeof(latestTele));
  newTeleAvailable = true;

  String payload = "{\"source\":\"vehicle\","
    "\"pitch\":" + String(latestTele.pitch, 2) +
    ",\"roll\":" + String(latestTele.roll, 2) +
    ",\"curA\":" + String(latestTele.curA) +
    ",\"curB\":" + String(latestTele.curB) +
    ",\"curC\":" + String(latestTele.curC) +
    ",\"curD\":" + String(latestTele.curD) +
    ",\"curE\":" + String(latestTele.curE) +
    ",\"isBalanced\":" + String(latestTele.isBalanced ? "true" : "false") + "}";

  if (mqtt.connected()) {
    mqtt.publish("vehicle/status", payload.c_str());
  }
  Serial.println("[ESP-NOW] " + payload);
}

void sendCmdToVehicle(int speed, int direction, bool stop) {
  CmdPacket cmd = {speed, direction, stop};
  esp_now_send(nodeMAC, (uint8_t *)&cmd, sizeof(cmd));
}

void initCanhTay() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);
  initVacuum();

  delay(500);
  pwm.begin();
  pwm.setPWMFreq(60);
  delay(200);

  writeServo(0, 330);
  writeServo(1, 150);
  writeServo(2, 300);
  writeServo(3, 410);

  sfInit();

  bool hasConfig = loadConfig();
  if (!hasConfig) {
    startCaptivePortal();
    while (portalMode) {
      handlePortal();
      delay(10);
    }
    return;
  }

  Serial.printf("[WIFI] Connecting to '%s'...\n", wifiCfg.ssid.c_str());
  WiFi.begin(wifiCfg.ssid.c_str(), wifiCfg.password.c_str());

  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 20) {
    delay(500);
    Serial.print(".");
    retry++;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\n[WIFI] That bai -> Captive Portal");
    clearSavedConfig();
    startCaptivePortal();
    while (true) {
      handlePortal();
      delay(10);
    }
  }

  Serial.println("\n[WIFI] IP: " + WiFi.localIP().toString());

  mqtt.setCallback(mqttCallback);

  server.on("/", []() { server.send_P(200, "text/html", DASHBOARD_HTML); });
  server.on("/app.css", []() { server.send_P(200, "text/css", DASHBOARD_CSS); });
  server.on("/app.js", []() { server.send_P(200, "application/javascript", DASHBOARD_JS); });
  server.on("/reset-config", HTTP_GET, []() {
    clearSavedConfig();
    server.send(200, "text/plain", "Da xoa config! Khoi dong lai...");
    delay(3000);
    ESP.restart();
  });
  server.begin();

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  esp_now_init();
  esp_now_register_recv_cb(OnDataRecv);
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, nodeMAC, 6);
  peer.channel = 0;
  peer.encrypt = false;
  esp_now_add_peer(&peer);

  Serial.println("[SETUP] Hoan tat!");
  printAngles();
}

void updateCanhTay() {
  if (portalMode) {
    handlePortal();
    return;
  }

  server.handleClient();
  webSocket.loop();

  mqttFsmTick();
  if (mqttGetState() == ROBOT_MQTT_CONNECTED) {
    mqtt.loop();
  }

  if (newTeleAvailable) {
    newTeleAvailable = false;
    broadcastTelemetry();
  }

  if (autoMode) {
    runAutoStep();
  }

  checkVacuumFailsafe();

  static unsigned long lastQueueBroadcast = 0;
  if (millis() - lastQueueBroadcast > 5000) {
    broadcastQueueStatus();
    lastQueueBroadcast = millis();
  }
}
