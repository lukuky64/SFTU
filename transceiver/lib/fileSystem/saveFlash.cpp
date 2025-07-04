#include "saveFlash.hpp"

SaveFlash::SaveFlash(SerialCom *serialCom) {
  m_serialCom = serialCom;  // Initialize the SerialCom pointer
  // Constructor implementation
}

void SaveFlash::begin() {
  if (!LittleFS.begin()) {
    ESP_LOGW(TAG, "LittleFS Mount Failed, attempting to format...");
    if (LittleFS.format()) {
      ESP_LOGI(TAG, "LittleFS formatted successfully");
      if (!LittleFS.begin()) {
        ESP_LOGE(TAG, "LittleFS Mount Failed even after format");
        return;
      }
    } else {
      ESP_LOGE(TAG, "LittleFS format failed");
      return;
    }
  }
  m_initialised = true;

  updateStorage();
  newLog();
}

void SaveFlash::newLog() {
  if (!m_initialised) {
    ESP_LOGW(TAG, "File system not initialised");
    return;
  }

  // Check if file exists and get its size
  File file = LittleFS.open(fileName, FILE_READ);
  if (!file) {
    // File doesn't exist, create it with "New Log"
    ESP_LOGI(TAG, "File doesn't exist, creating new log");
    writeData("New Log\n");
    return;
  }

  // Check if file is empty
  size_t fileSize = file.size();
  if (fileSize == 0) {
    file.close();
    ESP_LOGI(TAG, "File is empty, adding new log");
    writeData("New Log\n");
    return;
  }

  // Read the last line efficiently by seeking near the end
  String lastLine;
  file.seek(0, SeekEnd);  // Go to end of file

  // Read backwards to find the last line (simple approach: read last 50 bytes)
  size_t readSize = min(fileSize, (size_t)50);
  file.seek(fileSize - readSize);

  String endContent = file.readString();
  file.close();
  // Extract the last line
  int lastNewlineIndex = endContent.lastIndexOf('\n');
  if (lastNewlineIndex != -1) {
    // If there's content after the last newline, that's the last line
    if (lastNewlineIndex < endContent.length() - 1) {
      lastLine = endContent.substring(lastNewlineIndex + 1);
    } else {
      // The file ends with a newline, so find the second-to-last newline
      String beforeLastNewline = endContent.substring(0, lastNewlineIndex);
      int secondLastNewlineIndex = beforeLastNewline.lastIndexOf('\n');
      if (secondLastNewlineIndex != -1) {
        lastLine = beforeLastNewline.substring(secondLastNewlineIndex + 1);
      } else {
        // No second newline found, the entire content before last newline is
        // the last line
        lastLine = beforeLastNewline;
      }
    }
  } else {
    lastLine = endContent;  // No newline found, entire content is the last line
  }

  // Check if last line is already "New Log"
  lastLine.trim();  // Remove any whitespace
  if (lastLine.equals("New Log")) {
    ESP_LOGI(TAG, "Last line already 'New Log', skipping");
    return;
  }

  ESP_LOGI(TAG, "Adding new log entry");
  writeData("New Log\n");
}

void SaveFlash::writeData(const String &data) {
  if (!m_initialised) {
    ESP_LOGW(TAG, "File system not initialised");
    return;
  }

  if (data.length() < m_free) {
    File file = LittleFS.open(fileName, FILE_APPEND);
    if (!file) {
      ESP_LOGE(TAG, "Failed to open file for appending");
      return;
    }
    if (file.print(data)) {
      ESP_LOGD(TAG, "Data written successfully");
      m_free -= data.length();
    } else {
      ESP_LOGE(TAG, "Write failed");
    }
    file.close();
  } else {
    ESP_LOGE(TAG, "Not enough space to write data");
    return;
  }
}

void SaveFlash::removeFile() {
  if (!m_initialised) {
    ESP_LOGW(TAG, "File system not initialised");
    return;
  }

  if (LittleFS.remove(fileName)) {
    ESP_LOGI(TAG, "File removed successfully: %s", fileName);
    updateStorage();
  } else {
    ESP_LOGE(TAG, "Failed to remove file: %s", fileName);
  }
}

void SaveFlash::updateStorage() {
  if (!m_initialised) {
    ESP_LOGW(TAG, "File system not initialised");
    return;
  }

  m_total = LittleFS.totalBytes();
  m_used = LittleFS.usedBytes();
  m_free = m_total - m_used;

  ESP_LOGI(TAG, "\nTotal: %d bytes\nUsed: %d bytes\nFree: %d bytes", m_total,
           m_used, m_free);
}

void SaveFlash::readFile() {
  if (!m_initialised) {
    ESP_LOGW(TAG, "File system not initialised");
    return;
  }

  File file = LittleFS.open(fileName, FILE_READ);
  if (!file) {
    ESP_LOGE(TAG, "Failed to open file for reading");
    return;
  }
  ESP_LOGI(TAG, "Reading file: %s", fileName);
  m_serialCom->sendData("--------------------------------\n");
  while (file.available()) {
    String line = file.readStringUntil('\n');
    line += '\n';  // Add the newline back
    m_serialCom->sendData(line.c_str());
  }
  m_serialCom->sendData("--------------------------------\n");
  file.close();
}

// void SaveFlash::sendCommand(const char *command) {
//   if (!m_initialised) {
//     ESP_LOGW(TAG, "File system not initialised");
//     return;
//   }

//   if (strcmp(command, "read") == 0) {
//     readFile();
//   } else if (strcmp(command, "remove") == 0) {
//     removeFile();
//   } else if (strcmp(command, "update") == 0) {
//     updateStorage();
//   } else {
//     ESP_LOGW(TAG, "Unknown command: %s", command);
//   }
// }