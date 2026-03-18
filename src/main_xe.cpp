#include <Arduino.h>
#include "xe/xe.h"

void setup() {
  initXe();
}

void loop() {
  updateXe();
  delay(20);
}
