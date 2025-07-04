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

      return;
    }
    instance->RxFlag = true;
  }
}

void LoRaCom::sendMessage(const char *msg) {
  if (!RxFlag && radioInitialised) {
    if (msg[0] != '\0') {
      int state = radio->startTransmit(msg);
      instance->TxMode = true;
      if (state == RADIOLIB_ERR_NONE) {
        ESP_LOGI(TAG, "Transmitting: <%s>", msg);
      } else {
        ESP_LOGE(TAG, "Failed to begin transmission, code: %d", state);
      }
    }
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

int32_t LoRaCom::getRssi() {
  return radio->getRSSI();  // Return the last received signal strength
}

/* ================================ SETTERS ================================ */

bool LoRaCom::setOutGain(int8_t gain) {
  // value should be bewteen -9 and 22 dBm
  int state = radio->setOutputPower(gain);
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
