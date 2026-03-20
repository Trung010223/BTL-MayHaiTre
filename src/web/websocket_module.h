#pragma once

#include <WebSocketsServer.h>

void broadcastMotorState();
void broadcastTelemetry();
void broadcastQueueStatus();

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length);
