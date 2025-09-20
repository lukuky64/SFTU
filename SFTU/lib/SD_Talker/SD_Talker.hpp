#pragma once

#include <SD.h>

#include "Arduino.h"
#include "esp_log.h"

typedef struct {
  float value1;
  float value2;
  float value3;
  float value4;
  float value5;
  float value6;
  float value7;
  float value8;
  float battery_voltage;
  uint32_t timestamp;
} SampleWithTimestamp;

class SD_Talker {
 public:
  SD_Talker();
  ~SD_Talker();

#if DUMMY_SD

  bool checkStatus() { return true; }
  bool begin(uint8_t CS, SPICOM &SPI_BUS) { return true; }
  bool createFile(String StartMsg, String prefix) { return true; }
  bool writeToBuffer(String dataString) { return true; }
  void flushBuffer();
  bool isInitialized() { return true; }
  String createUniqueLogFile(String prefix) { return "true"; }
  bool createNestedDirectories(String prefix) { return true; }
  bool checkPresence() { return true; }
  bool writeBlockToSD(const float *block, size_t count) { return true; }

#else
  bool checkStatus();
  bool begin(uint8_t cardDetect, uint8_t CS, SPIClass &SPI_BUS);

  bool createFile(String StartMsg, String prefix);

  // bool writeToBuffer(String dataString);
  bool writeBuffer(const char *buffer, size_t bufferIndex);

  String createUniqueLogFile(String prefix);
  bool createNestedDirectories(String prefix);
  bool checkPresence();
  bool checkFileOpen();
  // New overload for value+timestamp
  bool writeBlockToSD(const SampleWithTimestamp *block, size_t count);
  // bool startNewLog(String filePrefix);
  bool startNewLog(String filePrefix, const std::vector<String> &channelNames, const std::vector<String> &channelUnits);

 private:
  File dataFile;
  String fileName;
  String buffer;
  bool m_initialised;
  bool m_fileOpen;

  uint8_t m_cardDetectPin;

  SPIClass *m_SPI_BUS = nullptr;
  uint8_t m_CS;

  bool sdWait(int timeout);

  static constexpr const char *TAG = "SD_Talker";

#endif
};