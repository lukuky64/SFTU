#pragma once

#include <Arduino.h>

#include <cstring>

#include "../pin_defs.hpp"
#include "LoRaCom.hpp"
#include "SerialCom.hpp"
#include "commander.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "saveFlash.hpp"

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

class Control
{
public:
  Control();
  void setup();
  void begin();

private:
  SerialCom *m_serialCom;
  LoRaCom *m_LoRaCom;
  Commander *m_commander;
  SaveFlash *m_saveFlash;

  unsigned long serial_Interval = 100;
  unsigned long lora_Interval = 100;
  unsigned long status_Interval = 5'100; // slightly out of sync with other device
  unsigned long heartBeat_Interval = 250;

  static constexpr const char *TAG = "Control";

  TaskHandle_t SerialTaskHandle = nullptr;
  TaskHandle_t LoRaTaskHandle = nullptr;
  TaskHandle_t StatusTaskHandle = nullptr;
  TaskHandle_t heartBeatTaskHandle = nullptr;

  void serialDataTask();
  void loRaDataTask();
  void statusTask();
  void heartBeatTask();

  // void interpretMessage(const char *buffer, bool relayMsgLoRa = true);
  void processData(const char *buffer);

  String deviceID = "transceiver"; // Unique identifier for the device

  // Mode of operation (transmit, receive, transceive, etc.)
  String m_mode = "transceive";
  String m_status = "ok";       // Status of the device (e.g., "ok", "error", etc.)
  float m_batteryLevel = 100.0; // Battery level as a percentage (0-100)

  // Data payload;
};