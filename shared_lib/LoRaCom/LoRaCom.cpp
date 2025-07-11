// LoRaCom.cpp
#include "LoRaCom.hpp"

LoRaCom::LoRaCom() {
  instance = this;  // Set the static instance pointer
  ESP_LOGI(TAG, "LoRaCom constructor called");
}

// void LoRaCom::setRxFlag() {
//   if (instance) {
//     instance->RxFlag = true;
//   }
// }

void LoRaCom::RxTxCallback(void) {
  if (instance) {
    if (instance->TxMode) {
      int state = instance->radio->finishTransmit();
      state |= instance->radio->startReceive();
      instance->TxMode = false;
      if (state == RADIOLIB_ERR_NONE) {
        ESP_LOGI(TAG, "Transmission finished");
      } else {
        ESP_LOGE(TAG, "Transmission failed, code: %d", state);
      }
    } else {
      instance->RxFlag = true;
    }
  }
}

void LoRaCom::sendMessage(const char *msg, uint32_t timeout_ms) {
  if (!radioInitialised || (msg[0] == '\0')) {
    return;
  }
  uint32_t startTick = xTaskGetTickCount();
  while (RxFlag) {
    vTaskDelay(pdMS_TO_TICKS(1));
    if ((xTaskGetTickCount() - startTick) > pdMS_TO_TICKS(timeout_ms)) {
      ESP_LOGE(TAG, "sendMessage timeout waiting for RxFlag to clear");
      return;
    }
  }

  int state = radio->startTransmit(msg);
  if (state == RADIOLIB_ERR_NONE) {
    instance->TxMode = true;
    ESP_LOGI(TAG, "Transmitting: <%s>", msg);
    while (TxMode) vTaskDelay(pdMS_TO_TICKS(1));  // Wait for transmission to finish
  } else {
    ESP_LOGE(TAG, "Failed to begin transmission, code: %d", state);
  }
}

bool LoRaCom::checkTxMode() {
  return TxMode;  // Return the current transmission mode status
}

bool LoRaCom::getMessage(char *buffer, size_t len) {
  if (RxFlag && radioInitialised) {
    int state = radio->readData(reinterpret_cast<uint8_t *>(buffer), len);
    RxFlag = false;
    state |= radio->startReceive();
    return (state == RADIOLIB_ERR_NONE);
  }
  return false;
}

bool LoRaCom::checkRx() { return RxFlag; }

int32_t LoRaCom::getRssi() {
  return radio->getRSSI();  // Return the last received signal strength
}

/* ================================ SETTERS ================================ */

bool LoRaCom::setOutGain(int8_t gain) {
  // value should be bewteen -9 and 22 dBm

  int state = RADIOLIB_ERR_NONE;

  if (radioType == RADIO_SX126X) {
    state |= static_cast<SX1262 *>(radio)->setOutputPower(gain);
  } else if (radioType == RADIO_SX127X) {
    state |= static_cast<SX1278 *>(radio)->setOutputPower(gain);
  }

  // int state = radio->setOutputPower(gain);
  if (state == RADIOLIB_ERR_NONE) {
    ESP_LOGI(TAG, "Gain set to %d", gain);
    return true;
  } else {
    ESP_LOGE(TAG, "Failed to set gain with code: %d", state);
    return false;
  }
}

bool LoRaCom::setFrequency(float freqMHz) {
  // Set the frequency of the radio
  int state = radio->setFrequency(freqMHz);
  if (state == RADIOLIB_ERR_NONE) {
    ESP_LOGI(TAG, "Frequency set to %.2f MHz", freqMHz);
    return true;
  } else {
    ESP_LOGE(TAG, "Failed to set frequency with code: %d", state);
    return false;
  }
}

bool LoRaCom::setSpreadingFactor(uint8_t spreadingFactor) {
  int state = RADIOLIB_ERR_UNKNOWN;
  if (!radio) {
    ESP_LOGE(TAG, "Radio pointer is null!");
    return false;
  }
  if (radioType == RADIO_SX127X) {
    state = static_cast<SX1278 *>(radio)->setSpreadingFactor(spreadingFactor);
  } else if (radioType == RADIO_SX126X) {
    state = static_cast<SX126x *>(radio)->setSpreadingFactor(spreadingFactor);
  } else {
    ESP_LOGE(TAG, "Unknown or unsupported radio type!");
    return false;
  }
  if (state == RADIOLIB_ERR_NONE) {
    ESP_LOGI(TAG, "Spreading factor set to %d", spreadingFactor);
    return true;
  } else {
    ESP_LOGE(TAG, "Failed to set spreading factor with code: %d", state);
    return false;
  }
}

bool LoRaCom::setBandwidth(float bandwidth) {
  int state = RADIOLIB_ERR_UNKNOWN;
  if (!radio) {
    ESP_LOGE(TAG, "Radio pointer is null!");
    return false;
  }
  if (radioType == RADIO_SX127X) {
    state = static_cast<SX1278 *>(radio)->setBandwidth(bandwidth);
  } else if (radioType == RADIO_SX126X) {
    state = static_cast<SX126x *>(radio)->setBandwidth(bandwidth);
  } else {
    ESP_LOGE(TAG, "Unknown or unsupported radio type!");
    return false;
  }
  if (state == RADIOLIB_ERR_NONE) {
    ESP_LOGI(TAG, "Bandwidth set to %.2f kHz", bandwidth);
    return true;
  } else {
    ESP_LOGE(TAG, "Failed to set bandwidth with code: %d", state);
    return false;
  }
}