// LoRaCom.h
#ifndef LoRaCom_h
#define LoRaCom_h

#include <Arduino.h>
#include <RadioLib.h>

#include "esp_log.h"

enum RadioType { RADIO_UNKNOWN, RADIO_SX127X, RADIO_SX126X };

class LoRaCom {
 public:
  LoRaCom();

  template <typename RadioType>
  bool begin(uint8_t CLK, uint8_t MISO, uint8_t MOSI, uint8_t csPin, uint8_t intPin, uint8_t RST, int8_t power, int8_t BUSY = -1) {
    SPI.begin(CLK, MISO, MOSI, csPin);

    radio = new RadioType((BUSY == -1) ? new Module(csPin, intPin, RST) : new Module(csPin, intPin, RST, BUSY));

    float freqMHz = 910.0f;        // Default frequency for LoRa <137.0 - 960.0> MHz
    float bw = 125.0f;             // Default bandwidth for LoRa <7.8 - 510.0> kHz
    int8_t sf = 9;                 // Spreading factor <5 - 12>
    uint8_t cr = 5;                // Coding rate denominator (4/cr) <5 - 8>
    uint8_t syncWord = 0x12;       // sync word for private LoRa
    uint16_t preambleLength = 16;  // preamble length in symbols

    int state = RADIOLIB_ERR_NONE;

    state |= static_cast<RadioType *>(radio)->begin(freqMHz, bw, sf, cr, syncWord, power, preambleLength);

    state |= static_cast<RadioType *>(radio)->forceLDRO(true);

    if (radioType == RADIO_SX126X) {
      state |= static_cast<SX1262 *>(radio)->setRegulatorLDO();
      state |= static_cast<SX1262 *>(radio)->setCurrentLimit(140);  // set max current limit to 140 mA
      state |= static_cast<SX1262 *>(radio)->calibrateImage(freqMHz);
      state |= static_cast<SX1262 *>(radio)->setRxBoostedGainMode(true, true);
    }

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

  void setRadioType(RadioType type) { radioType = type; }

  void sendMessage(const char *msg, uint32_t timeout_ms);  // overloaded function
  bool getMessage(char *buffer, size_t len);
  bool checkRx();
  int32_t getRssi();

  bool setOutGain(int8_t gain);
  bool setFrequency(float freqMHz);

  // not supported for the physical layer
  bool setSpreadingFactor(uint8_t spreadingFactor);
  bool setBandwidth(float bandwidth);

  bool checkTxMode();

 private:
  PhysicalLayer *radio;
  RadioType radioType = RADIO_UNKNOWN;
  inline static LoRaCom *instance = nullptr;

  bool radioInitialised = false;

  volatile bool RxFlag = false;

  volatile bool TxMode = false;

  static void RxTxCallback(void);

  static constexpr const char *TAG = "LORA_COMM";
};

#endif