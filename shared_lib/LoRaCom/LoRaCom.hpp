// LoRaCom.h
#ifndef LoRaCom_h
#define LoRaCom_h

#include <Arduino.h>
#include <RadioLib.h>

#include "LoRaMsg.hpp"
#include "esp_log.h"

// https://www.semtech.com/design-support/lora-calculator

#define BROADCAST_ID 0xFF

enum RadioType { RADIO_UNKNOWN, RADIO_SX127X, RADIO_SX126X };

class LoRaCom {
 public:
  LoRaCom();

  template <typename RadioType>
  bool begin(uint8_t CLK, uint8_t MISO, uint8_t MOSI, uint8_t csPin, uint8_t intPin, uint8_t RST, int8_t power, int8_t BUSY = -1) {
    SPI.begin(CLK, MISO, MOSI, csPin);

    radio = new RadioType((BUSY == -1) ? new Module(csPin, intPin, RST) : new Module(csPin, intPin, RST, BUSY));

    float freqMHz = 910.0f;        // Default frequency for LoRa <137.0 - 960.0> MHz
    float bw = 500.0f;             // Default bandwidth for LoRa <7.8 - 500.0> kHz. Each increment changes sensitivity by ~3dB
    int8_t sf = 7;                 // Spreading factor <5 - 12>. Each increment changes sensitivity by 2.5-3.0dB. High = better
    uint8_t cr = 5;                // Coding rate denominator (4/cr) <5 - 8>
    uint8_t syncWord = 0x12;       // sync word for private LoRa
    uint16_t preambleLength = 16;  // preamble length in symbols. delta_sensitivity = 10*log(preambleLength=16/8) = 3dB increase

    int state = RADIOLIB_ERR_NONE;

    state |= static_cast<RadioType *>(radio)->begin(freqMHz, bw, sf, cr, syncWord, power, preambleLength);

    // state |= static_cast<RadioType *>(radio)->forceLDRO(true);  // ! test this
    state |= static_cast<RadioType *>(radio)->autoLDRO();  // Enable automatic low data rate optimization

    if (radioType == RADIO_SX126X) {
      // not sure if this is better, I imagine less interference over SMPS. Semtech calculator shows no difference
      // state |= static_cast<SX1262 *>(radio)->setRegulatorLDO();
      // set max current limit to 140 mA
      state |= static_cast<SX1262 *>(radio)->setCurrentLimit(140);
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

  bool sendMessage(const uint8_t *data, size_t len, uint32_t timeout_ms, int queueIndex = -1);

  bool getMessage(LoRaMessage *msg);
  bool checkRx();
  int32_t getRssi();

  bool setOutGain(int8_t gain);
  bool setFrequency(float freqMHz);

  bool setSpreadingFactor(uint8_t spreadingFactor);
  bool setBandwidth(float bandwidth);

  bool checkTxMode();

  bool enqueueMessage(LoRaMessage &msg, bool requireAck = false);
  void processSendQueue();

  bool isAcked(uint8_t seqID);
  bool isFailed(uint8_t seqID);
  bool isQueued(uint8_t seqID);

  bool stringToCommandPayload(CommandPayload &payload, const char *buffer);

 private:
  int currentTxIndex = -1;  // Track which message is being transmitted
  static void RxTxCallback(void);

  void handleAck(uint16_t ackSeqID);
  bool receiveMessage(LoRaMessage *msg);
  void compactSendQueue();
  void sendAck(uint8_t targetID, uint8_t seqID);
  void moveToDoneQueue(const QueuedMessage &q);

  PhysicalLayer *radio;
  RadioType radioType = RADIO_UNKNOWN;
  inline static LoRaCom *instance = nullptr;

  bool radioInitialised = false;
  volatile bool RxFlag = false;
  volatile bool TxMode = false;

  QueuedMessage sendQueue[MAX_QUEUE_SIZE];
  uint8_t sendHead = 0, sendTail = 0, sendCount = 0;

  QueuedMessage doneQueue[MAX_QUEUE_SIZE];
  uint8_t doneHead = 0, doneTail = 0, doneCount = 0;

  uint8_t nextSequenceID = 0;

  static constexpr const char *TAG = "LORA_COMM";
};

#endif