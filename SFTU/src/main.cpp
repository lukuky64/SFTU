#include <Arduino.h>

#include "control.hpp"
#include "esp_log.h"

Control *control = nullptr;

void setup() {
  ESP_LOGI("Main", "Starting setup...");
  esp_reset_reason_t reason = esp_reset_reason();
  ESP_LOGI("Main", "Reset reason: %d", reason);

  control = new Control();
  control->setup();
  vTaskDelay(pdMS_TO_TICKS(2'000));
  control->begin();
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(10'000));  // delay to allow tasks to run
}
