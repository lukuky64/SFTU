#pragma once

#include <Arduino.h>

#include "esp_log.h"

#define COMM_INTERFACE Serial

class SerialCom {
 public:
  SerialCom();

  void init(unsigned long baud = 115200);

  bool getData(char *buffer, const size_t bufferSize, int *_rxIndex);

  void sendData(const char *data);

 private:
  unsigned long m_baud;

  static constexpr const char *TAG = "SerialCom";
};
