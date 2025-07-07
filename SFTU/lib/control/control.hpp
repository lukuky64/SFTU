#pragma once

#include <Arduino.h>

#include <cstring>

#include "../Pin_Defs.hpp"
#include "LoRaCom.hpp"
#include "SerialCom.hpp"
#include "Wire.h"
#include "adcADS.hpp"
#include "commander.hpp"
#include "driver/timer.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifdef SFTU
#include "SD_Talker.hpp"
#include "actuation.hpp"

#else
#include "saveFlash.hpp"
#endif

#define c_cmp(a, b) (strcmp(a, b) == 0)

// this will be the data that is sent over LoRa in both directions on loop

// eg: "command update gain 22"
// eg: "status <deviceID> <RSSI> <batteryLevel> <mode> <status>"
// eg: "data <payload>"

// struct Data {
//   float force;
//   float timestamp;
// };

// struct Data {
//   String DataType;  // Type of data (e.g., "command", "data", "message")
//   String payload;   // The actual data being sent
//   int32_t RSSI;     // Received Signal Strength Indicator
//   String mode;      // mode of device (freq sweep receive, transceive,
//   transmit,
//                     // recieve, etc)
//   int8_t status;  // Status of the operation (e.g., all ok, error, warning,
//   low
//                   // battery, etc)
//   float batteryLevel;  // Battery level in volts
// };

class Control {
 public:
  Control();
  void setup();
  void begin();

 private:
  TwoWire *m_I2C_BUS;
  TwoWire *m_ANALOG_I2C_BUS;

  SPIClass *m_SPI_BUS;

  SerialCom *m_serialCom;
  LoRaCom *m_LoRaCom;
  Commander *m_commander;
  SD_Talker *m_sdTalker;

#ifdef SFTU
  Actuation *m_actuation;
#else
  SaveFlash *m_saveFlash;
#endif

  adcADS *m_adcADS;

  unsigned long serial_Interval = 100;
  unsigned long lora_Interval = 100;
  unsigned long status_Interval = 2'000;
  unsigned long heartBeat_Interval = 500;

  static constexpr const char *TAG = "Control";

  TaskHandle_t SerialTaskHandle = nullptr;
  TaskHandle_t LoRaTaskHandle = nullptr;
  TaskHandle_t StatusTaskHandle = nullptr;
  TaskHandle_t heartBeatTaskHandle = nullptr;
  TaskHandle_t analogTaskHandle = nullptr;
  TaskHandle_t sdTaskHandle = nullptr;

  void serialDataTask();
  void loRaDataTask();
  void statusTask();
  void heartBeatTask();
  void analogTask();
  void sdTask();

  volatile bool adcSampleFlag = false;

  void interpretMessage(const char *buffer, bool relayMsgLoRa);
  void processData(const char *buffer);
  void queueSample();

  String deviceID = "transceiver";  // Unique identifier for the device

  // Mode of operation (transmit, receive, transceive, etc.)
  String m_mode = "transceive";
  String m_status = "ok";  // Status of the device (e.g., "ok", "error", etc.)
  float m_batteryLevel = 100.0;  // Battery level as a percentage (0-100)

  xQueueHandle m_adcQueue;
  uint16_t adcSPS = 860;  // Set the data rate for the ADC

  // Data payload;
};