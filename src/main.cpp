#include <Arduino.h>
#include "arm/core/canh_tay.h"
#include "vehicle/core/xe.h"

// Khởi động tất cả modules
void setup() {
  initCanhTay();
  initXe();
}

// Loop chính - gọi các hàm update từ modules
void loop() {
  updateCanhTay();
  updateXe();
  delay(20);
}

