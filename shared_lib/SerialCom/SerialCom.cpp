#include "SerialCom.hpp"

SerialCom::SerialCom() {}

void SerialCom::init(unsigned long baud) {
  m_baud = baud;
  COMM_INTERFACE.begin(m_baud);
  ESP_LOGI(TAG, "Serial communication initialised at %lu baud", m_baud);
}

bool SerialCom::getData(char *buffer, const size_t maxBuffer, int *_rxIndex) {
  // Read all available characters
  while (COMM_INTERFACE.available()) {
    char c = COMM_INTERFACE.read();

    // If it's a newline (end of command), handle the buffer
    if (c == '\n' || c == '\r') {
      // Null-terminate the buffer and process
      buffer[*_rxIndex] = '\0';
      return true;
    } else {
      // Add the character to the buffer (with overflow check)
      if (*_rxIndex < (maxBuffer - 1)) {
        buffer[(*_rxIndex)++] = c;
      } else {
        // Buffer overflow case, reset it
        *_rxIndex = 0;
        ESP_LOGE(TAG, "Buffer overflow!");
        break;
      }
    }
  }
  return false;
}

void SerialCom::sendData(const char *data) {
  COMM_INTERFACE.print(data);
  // ESP_LOGI(TAG, "Sent: %s", data);
}