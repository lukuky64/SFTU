#include "SD_Talker.hpp"

#if DUMMY_SD
SD_Talker::SD_Talker() {}

SD_Talker::~SD_Talker() {}

#else

SD_Talker::SD_Talker() : m_fileOpen(false), m_initialised(false) {}

SD_Talker::~SD_Talker() {
  // Ensure the file is closed and buffer is flushed upon object destruction
  // flushBuffer();
  if (m_fileOpen) {
    dataFile.close();
  }
}

bool SD_Talker::checkStatus() {
  if (!m_initialised) {
    ESP_LOGE("SD_Talker", "SD card not initialised.");
    return false;
  }

  // Check if SD card is connected
  bool sdStatus = checkPresence();

  // bool sdStatus = sdWait(50); // Check drive 0, wait up to 25 ms. Won't need
  // this if we have card detect pin, although this checks communication

  if (!sdStatus) {
    ESP_LOGE("SD_Talker", "SD card not connected.");
    return false;
  } else {
    // ESP_LOGI("SD_Talker", "SD card connected. Type: %d", cardType);
    return true;
  }
}

bool SD_Talker::checkPresence() {
  bool cardPresent = !digitalRead(m_cardDetectPin);

  if (!cardPresent) {
    if (m_fileOpen) {
      dataFile.close();
      SD.end();
      m_fileOpen = false;
    }
  }

  return cardPresent;
}

bool SD_Talker::checkFileOpen() { return m_fileOpen; }

// seems to be working
bool SD_Talker::sdWait(int timeout) {
  uint8_t response;
  uint32_t start = millis();
  // Send some dummy clocks to ensure the card is ready. CS is high so no data
  // is sent
  digitalWrite(m_CS, HIGH);
  for (int i = 0; i < 10; i++) {
    m_SPI_BUS->transfer(0xFF);
  }

  // Assert CS to select the SD card
  digitalWrite(m_CS, LOW);

  // Send CMD13: 0x40 OR 13, then four bytes argument (all zeros) and dummy CRC
  m_SPI_BUS->transfer(0x40 | 13);  // CMD13 command token
  m_SPI_BUS->transfer(0x00);       // Argument byte 1
  m_SPI_BUS->transfer(0x00);       // Argument byte 2
  m_SPI_BUS->transfer(0x00);       // Argument byte 3
  m_SPI_BUS->transfer(0x00);       // Argument byte 4
  m_SPI_BUS->transfer(0x01);       // Dummy CRC

  // Wait for the response (CMD13 returns an R2 response, which is 2 bytes; here
  // we just get the first byte)
  do {
    response = m_SPI_BUS->transfer(0xFF);
  } while ((response & 0x80) && ((millis() - start) < (unsigned int)timeout));

  // Deselect the SD card
  digitalWrite(m_CS, HIGH);

  // ESP_LOGI("SD_Talker", "CMD13 Response: 0x%02X", response);
  return (response != 0x00);  // A response other than 0xFF indicates the card
                              // replied. But seems 0x00 means it failed
}

bool SD_Talker::begin(uint8_t cardDetect, uint8_t CS, SPIClass &SPI_BUS) {
  m_initialised = false;

  m_SPI_BUS = &SPI_BUS;
  m_CS = CS;
  m_cardDetectPin = cardDetect;

  pinMode(m_cardDetectPin, INPUT);

  if (checkPresence()) {
    // See if the card is present and can be initialized:
    if (SD.begin(m_CS, *m_SPI_BUS)) {
      m_initialised = true;
    }
  }

  ESP_LOGD(TAG, "SD card initialised: %s", m_initialised ? "true" : "false");

  return m_initialised;
}

bool SD_Talker::createNestedDirectories(String prefix) {
  if (!m_initialised || !checkPresence() || prefix.isEmpty()) {
    ESP_LOGE(TAG, "SD card not initialised or prefix is empty.");
    return false;
  }

  bool success = true;

  // Remove leading slash for consistent path building
  String cleanPrefix = prefix;
  if (cleanPrefix.startsWith("/")) {
    cleanPrefix = cleanPrefix.substring(1);
  }

  int lastPos = 0;
  int pos = cleanPrefix.indexOf('/');
  String currentPath = "";

  while (pos != -1) {
    currentPath += "/" + cleanPrefix.substring(lastPos, pos);
    if (!SD.exists(currentPath)) {
      if (!SD.mkdir(currentPath)) {
        success = false;
        break;
      }
    }
    lastPos = pos + 1;
    pos = cleanPrefix.indexOf('/', lastPos);
  }

  // Create the final directory if not already created and not empty
  if (success && lastPos < cleanPrefix.length()) {
    currentPath += "/" + cleanPrefix.substring(lastPos);
    if (!SD.exists(currentPath)) {
      if (!SD.mkdir(currentPath)) {
        success = false;
      }
    }
  }

  return success;
}

bool SD_Talker::createFile(String StartMsg, String prefix) {
  bool success = false;

  if (!m_fileOpen) {
    // begin still needs have been called before this
    if (checkPresence()) {
      vTaskDelay(pdMS_TO_TICKS(1000));

      // delay to allow card to be fully installed
      bool began = begin(m_cardDetectPin, m_CS, *m_SPI_BUS);
      if (!began) {
        return false;
      }
    } else {
      return false;
    }

    // Only create directories for the parent folder, not the full prefix
    String cleanPrefix = prefix;
    if (cleanPrefix.endsWith("/")) {
      cleanPrefix = cleanPrefix.substring(0, cleanPrefix.length() - 1);
    }
    int lastSlash = cleanPrefix.lastIndexOf('/');
    String dir;
    if (lastSlash == -1) {
      dir = "";
    } else {
      dir = cleanPrefix.substring(0, lastSlash);
    }
    if (dir.length() > 0) {
      createNestedDirectories(dir);
    }

    fileName = createUniqueLogFile(prefix);

    {
      dataFile = SD.open(fileName.c_str(), FILE_WRITE);
      if (dataFile) {
        dataFile.println(StartMsg);
        dataFile.flush();

        ESP_LOGI(TAG, "Created file: %s", fileName.c_str());
        m_fileOpen = true;
        success = true;
      } else {
        success = false;
      }
    }
  }

  return success;
}

bool SD_Talker::writeBuffer(const char *buffer, size_t bufferIndex) {
  if (m_fileOpen) {
    size_t bytesWritten = dataFile.write((const uint8_t *)buffer, bufferIndex);
    dataFile.flush();
    if (bytesWritten != bufferIndex) {
      ESP_LOGE("SD_Talker", "Failed to write all bytes to SD card.");
      return false;
    } else {
      // ESP_LOGI("SD_Talker", "Successfully wrote %d bytes to SD card.",
      // bytesWritten);
      return true;
    }
  } else {
    ESP_LOGE("SD_Talker", "Attempted to write to SD card, but file is not open.");
    return false;
  }
}

String SD_Talker::createUniqueLogFile(String prefix) {
  // Remove trailing slash if present
  String cleanPrefix = prefix;
  if (cleanPrefix.endsWith("/")) {
    cleanPrefix = cleanPrefix.substring(0, cleanPrefix.length() - 1);
  }
  // Find last slash to separate directory and base name
  int lastSlash = cleanPrefix.lastIndexOf('/');
  String dir, base;
  if (lastSlash == -1) {
    dir = "";
    base = cleanPrefix;
  } else {
    dir = cleanPrefix.substring(0, lastSlash);
    base = cleanPrefix.substring(lastSlash + 1);
  }
  uint32_t currentLogIndex = 0;
  const uint32_t maxIterations = 1000;
  String uniqueFileName;
  do {
    if (currentLogIndex >= maxIterations) {
      return "";
    }
    if (dir.length() > 0) {
      uniqueFileName = dir + "/" + base + "_" + String(currentLogIndex++) + ".csv";
    } else {
      uniqueFileName = base + "_" + String(currentLogIndex++) + ".csv";
    }
  } while (SD.exists(uniqueFileName.c_str()));
  return uniqueFileName;
}

// New overload for value+timestamp
bool SD_Talker::writeBlockToSD(const SampleWithTimestamp *block, size_t count) {
  if (!m_fileOpen) {
    ESP_LOGE("SD_Talker", "Attempted to write to SD card, but file is not open.");
    return false;
  }

  if (!checkPresence()) {
    return false;
  }

  // Buffer the entire block as a String and write in one go
  String buffer;
  buffer.reserve(count * 140);  // Increased estimate for battery voltage column
  for (size_t i = 0; i < count; ++i) {
    char line[144];  // HACK: Increased buffer size for battery voltage
    snprintf(line, sizeof(line), "%llu,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f\n", (unsigned long long)block[i].timestamp, block[i].value1, block[i].value2, block[i].value3, block[i].value4, block[i].value5, block[i].value6, block[i].value7, block[i].value8, block[i].battery_voltage);
    buffer += line;
  }
  size_t bytesWritten = dataFile.print(buffer);
  dataFile.flush();
  if (bytesWritten != buffer.length()) {
    ESP_LOGE("SD_Talker", "Failed to write all bytes to SD card (block write).");
    return false;
  }
  return true;
}

bool SD_Talker::startNewLog(String filePrefix, const std::vector<String> &channelNames, const std::vector<String> &channelUnits) {
  if (!m_initialised || !checkPresence()) {
    return false;
  }

  // TODO: Set these with config file
  String startMsg = "Time(us)";
  for (size_t i = 0; i < channelNames.size(); ++i) {
    startMsg += ", " + channelNames[i] + "(" + channelUnits[i] + ")";
  }

  if (createFile(startMsg, filePrefix)) {
    ESP_LOGI(TAG, "Created file on SD card!");
    return true;
  } else {
    return false;
  }
}

#endif
