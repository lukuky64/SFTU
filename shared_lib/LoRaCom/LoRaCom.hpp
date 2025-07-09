// LoRaCom.h
#ifndef LoRaCom_h
#define LoRaCom_h

#include <Arduino.h>
#include <RadioLib.h>

#include "esp_log.h"

class LoRaCom {
 public:
  LoRaCom();

  template <typename RadioType>
  bool begin(uint8_t CLK, uint8_t MISO, uint8_t MOSI, uint8_t csPin,
             uint8_t intPin, uint8_t RST, float freqMHz, int8_t power,
             int8_t BUSY = -1) {
    SPI.begin(CLK, MISO, MOSI, csPin);

    radio = new RadioType((BUSY == -1) ? new Module(csPin, intPin, RST)
                                       : new Module(csPin, intPin, RST, BUSY));

    int state = static_cast<RadioType *>(radio)->begin(freqMHz, 500, 7, 5, 0x34,
                                                       power, 20);

    radio->setPacketReceivedAction(RxTxCallback);
    // radio->setPacketSentAction(TxCallback);

    state |= radio->startReceive();
    if (state == RADIOLIB_ERR_NONE) {
      ESP_LOGI(TAG, "LoRa initialised successfully!");
      radioInitialised = true;
      return true;
    } else {
      ESP_LOGE(TAG, "LoRa initialisation FAILED! Code: %d", state);
      return false;
    }
  }

  void sendMessage(const char *msg);  // overloaded function
  bool getMessage(char *buffer, size_t len);
  bool checkRx();
  int32_t getRssi();

  bool setOutGain(int8_t gain);
  bool setFrequency(float freqMHz);

  // not supported for the physical layer
  bool setSpreadingFactor(uint8_t spreadingFactor) { return false; }
  bool setBandwidth(float bandwidth) { return false; }

  bool checkTxMode();

 private:
  PhysicalLayer *radio;
  inline static LoRaCom *instance = nullptr;

  bool radioInitialised = false;

  volatile bool RxFlag = false;

  volatile bool TxMode = false;

  static void RxTxCallback(void);

  static constexpr const char *TAG = "LORA_COMM";
};

#endif