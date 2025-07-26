#pragma once

#include <Arduino.h>

#include <cstring>

#include "Definitions.hpp"
#include "LoRaCom.hpp"
#include "SerialCom.hpp"
#include "Wire.h"
#include "adcADS.hpp"
#include "commander.hpp"
#include "display.hpp"
#include "driver/timer.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifdef SFTU
#include "ADCprocessing.hpp"
#include "BattMonitor.hpp"
#include "SD_Talker.hpp"
#include "actuation.hpp"
#include "esp_task_wdt.h"

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
  Display *m_display;
  BattMonitor *m_battMonitor;
  ADCprocessing *m_adcProcessing;

#ifdef SFTU
  Actuation *m_actuation;
#else
  SaveFlash *m_saveFlash;
#endif
  // these need mutexes

  // There are 2 ADCs, each with 2 differential channels
  adcADS *m_adcADS_12;
  adcADS *m_adcADS_34;

  unsigned long serial_Interval = 50;
  unsigned long lora_Interval = 50;
  unsigned long status_Interval = 2'000;
  unsigned long heartBeat_Interval = 500;

  static constexpr const char *TAG = "Control";

  struct handles {
    TaskHandle_t SerialTaskHandle = nullptr;
    TaskHandle_t LoRaTaskHandle = nullptr;
    TaskHandle_t StatusTaskHandle = nullptr;
    TaskHandle_t heartBeatTaskHandle = nullptr;
    TaskHandle_t analogTaskHandle = nullptr;
    TaskHandle_t sdTaskHandle = nullptr;
    TaskHandle_t displayTaskHandle = nullptr;
  };

  handles m_taskHandles;

  struct HandleMap {
    String name;
    TaskHandle_t *handle;
  };

  HandleMap m_taskHandleMap[7] = {
      {"SerialTaskHandle", &m_taskHandles.SerialTaskHandle}, {"LoRaTaskHandle", &m_taskHandles.LoRaTaskHandle}, {"StatusTaskHandle", &m_taskHandles.StatusTaskHandle},   {"heartBeatTaskHandle", &m_taskHandles.heartBeatTaskHandle},
      {"analogTaskHandle", &m_taskHandles.analogTaskHandle}, {"sdTaskHandle", &m_taskHandles.sdTaskHandle},     {"displayTaskHandle", &m_taskHandles.displayTaskHandle},
  };

  void serialDataTask();
  void loRaDataTask();
  void statusTask();
  void heartBeatTask();
  void analogTask();
  void sdTask();
  void displayTask();
  void checkTaskStack();

  void setLatestSample(const SampleWithTimestamp &sample);
  void getLatestSample(SampleWithTimestamp &sample);

  volatile bool adcSampleFlag = false;

  // void interpretMessage(const char *buffer, bool relayMsgLoRa);
  void processData(const char *buffer);
  void queueSample();

  String deviceID = "SFTU";  // Unique identifier for the device

  // Mode of operation (transmit, receive, transceive, etc.)
  String m_mode = "transceive";
  String m_status = "ok";  // Status of the device (e.g., "ok", "error", etc.)
  float m_batteryVoltage = 0;
  volatile bool m_pauseADC = false;  // Flag to pause ADC sampling

  xQueueHandle m_adcQueue;

  SemaphoreHandle_t m_latestSampleMutex = nullptr;
  SampleWithTimestamp m_latestSample;

  // Data payload;
};