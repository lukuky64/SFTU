#include <Arduino.h>

#include "control.hpp"
#include "esp_log.h"

Control* control = nullptr;

void setup() {
  delay(1000);
  ESP_LOGI("Main", "Starting setup...");
  control = new Control();
  control->setup();
  control->begin();
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(10000));  // random delay to allow tasks to run
}