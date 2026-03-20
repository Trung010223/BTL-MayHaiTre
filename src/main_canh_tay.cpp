#include <Arduino.h>
#include "canh_tay/canh_tay.h"

void setup() {
  initCanhTay();
}

void loop() {
  updateCanhTay();
  delay(20);
}
