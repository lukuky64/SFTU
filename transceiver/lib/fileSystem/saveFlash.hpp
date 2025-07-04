#include <Arduino.h>

#include "LittleFS.h"
#include "SerialCom.hpp"
#include "esp_log.h"

class SaveFlash {
 public:
  SaveFlash(SerialCom *serialCom);
  void begin();
  void newLog();
  void writeData(const String &data);
  void removeFile();
  void readFile();
  void updateStorage();
  //   void sendCommand();

 private:
  SerialCom *m_serialCom;  // Pointer to SerialCom instance

  const char *TAG = "SaveFlash";
  const char *fileName = "/log.txt";
  bool m_initialised = false;

  size_t m_used = 0;
  size_t m_total = 0;
  size_t m_free = 0;
};