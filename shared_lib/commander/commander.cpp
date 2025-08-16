#include "commander.hpp"

#ifdef SFTU
Commander::Commander(SerialCom *serialCom, LoRaCom *loraCom, Actuation *actuation, adcADS *adcADS, loadCellProcessing *loadCellProcessing) {
  memset(m_command, 0, sizeof(m_command));
  m_serialCom = serialCom;
  m_loraCom = loraCom;
  m_actuation = actuation;
  m_adcADS = adcADS;
  m_loadCellProcessing = loadCellProcessing;
  ESP_LOGD(TAG, "Commander initialised");
}
#else
Commander::Commander(SerialCom *serialCom, LoRaCom *loraCom) {
  memset(m_command, 0, sizeof(m_command));  // Initialize command buffer
  m_serialCom = serialCom;                  // Initialize the SerialCom instance
  m_loraCom = loraCom;                      // Initialize the LoRaCom instance
  ESP_LOGD(TAG, "Commander initialised");
}
#endif

void Commander::handle_command_help() {
  handle_help(command_handler);  // Call the generic help handler
}

void Commander::handle_update_help() {
  handle_help(update_handler);  // Call the generic help handler
}

void Commander::handle_set_help() {
  handle_help(set_handler);  // Call the generic help handler
}

void Commander::handle_help(const HandlerMap *handler) {
  String helpText = "\nAvailable commands:\n";
  for (const HandlerMap *cmd = handler; cmd->name != nullptr; ++cmd) {
    helpText += "- <" + String(cmd->name) + ">\n";  // Append command names to help text
  }
  ESP_LOGI(TAG, "%s", helpText.c_str());
}

void Commander::handle_update() {
  ESP_LOGD(TAG, "Update command executed");
  checkCommand(update_handler);
}

void Commander::handle_set() {
  ESP_LOGD(TAG, "Set command executed");
  checkCommand(set_handler);  // Check and run the set command
}

void Commander::handle_update_gain() {
  ESP_LOGD(TAG, "Update gain command executing");

  char *data = readAndRemove();  // Read and remove the command token

  // convert to integer
  if (data == nullptr) {
    ESP_LOGW(TAG, "Empty data received for gain update, expecting <int8_t>");
    return;
  }

  int8_t gain = static_cast<int8_t>(atoi(data));  // Convert to int8_t
  m_loraCom->setOutGain(gain);                    // Set the gain in LoRaCom
}

void Commander::handle_update_freqMhz() {
  ESP_LOGD(TAG, "Update freqMhz command executing");

  char *data = readAndRemove();  // Read and remove the command token

  // convert to integer
  if (data == nullptr) {
    ESP_LOGW(TAG, "Empty data received for gain update, expecting <float>");
    return;
  }

  float freqMhz = static_cast<float>(atof(data));  // Cast to float
  m_loraCom->setFrequency(freqMhz);                // Set the gain in LoRaCom
}

void Commander::handle_update_spreadingFactor() {
  // Implementation for updating spreading factor
  ESP_LOGD(TAG, "Update spreading factor command executing");

  char *data = readAndRemove();  // Read and remove the command token

  // convert to integer
  if (data == nullptr) {
    ESP_LOGW(TAG, "Empty data received for gain update, expecting <uint8_t>");
    return;
  }

  uint8_t spreadingFactor = static_cast<uint8_t>(atoi(data));  // Cast to float
  m_loraCom->setSpreadingFactor(spreadingFactor);              // Set the gain in LoRaCom
}

void Commander::handle_update_bandwidthKHz() {
  // Implementation for updating bandwidth
  ESP_LOGD(TAG, "Update bandwidth command executing");

  char *data = readAndRemove();  // Read and remove the command token

  // convert to integer
  if (data == nullptr) {
    ESP_LOGW(TAG, "Empty data received for gain update, expecting <float>");
    return;
  }

  float bandwidthKhz = static_cast<float>(atof(data));  // Cast to float
  m_loraCom->setBandwidth(bandwidthKhz);                // Set the gain in LoRaCom
}
#ifdef SFTU
// void Commander::handle_set_OUTPUT() {
//   ESP_LOGD(TAG, "Set output command executing");
//   char *data = readAndRemove();  // Read and remove the command token

//   // convert to integer
//   if (data == nullptr) {
//     ESP_LOGW(TAG, "Empty data received");
//     return;
//   }

//   int outputIndex = atoi(data);  // Convert to 1-based index

//   // Validate output index range
//   if (outputIndex < 1 || outputIndex > 8) {
//     ESP_LOGW(TAG, "Invalid output index %d, must be 1-8", outputIndex);
//     return;
//   }

//   uint8_t outputPin = PCA6408A_outputPins[outputIndex];

//   data = readAndRemove();  // Read and remove the command token

//   // convert to integer
//   if (data == nullptr) {
//     ESP_LOGW(TAG, "Empty data received");
//     return;
//   }

//   bool ioState = ((atoi(data)) == 1) ? true : false;

//   ESP_LOGI(TAG, "Setting output to %s", ioState ? "ON" : "OFF");
//   m_actuation->setDigital(outputPin, (ioState ? OUTPUT_LOW : OUTPUT_OPEN));
// }

// void Commander::handle_calibrateCell() {
//   ESP_LOGD(TAG, "Calibrate cell command executing");

//   char *data = readAndRemove();  // Read and remove the command token

//   // convert to integer
//   if (data == nullptr) {
//     ESP_LOGW(TAG, "Empty data received for calibration, expecting <float>");
//     return;
//   }

//   float objectMass = static_cast<float>(atof(data));  // Cast to float

//   // TODO: We need to select the MUX
//   float averageVoltage = m_adcADS->getAverageVolt(100, ADS1X15_REG_CONFIG_MUX_DIFF_0_1);  // Read average voltage from ADC

//   m_loadCellProcessing->calibrate(objectMass, averageVoltage);
// }

// #else
// void Commander::handle_set_OUTPUT() { ESP_LOGD(TAG, "Command not implemented for this build"); }

// void Commander::handle_calibrateCell() { ESP_LOGD(TAG, "Command not implemented for this build"); }
#endif

void Commander::handle_mode() {
  // Implementation for mode command
  ESP_LOGD(TAG, "Mode command not implemented yet");
}

void Commander::checkCommand(const HandlerMap *handler_) {
  char *token = readAndRemove();
  if (token != nullptr) {
    runMappedCommand(token, handler_);  // Run the mapped command
  } else {
    ESP_LOGW(TAG,
             "No command provided, type <help> after action for a list of "
             "commands. eg: <command help>, <command update help>, etc.");
  }
}

void Commander::runMappedCommand(char *command, const HandlerMap *handler) {
  // Call the mapped command handler
  for (const HandlerMap *cmd = handler; cmd->name != nullptr; ++cmd) {
    if (c_cmp(command, cmd->name)) {
      return (this->*cmd->handler)();  // Call the corresponding handler
    }
  }
}

char *Commander::readAndRemove() {
  if (m_command == nullptr || *m_command == nullptr) return nullptr;

  char *start = *m_command;

  // Skip leading spaces
  while (*start == ' ') start++;

  // If we reached the end, return nullptr
  if (*start == '\0') {
    *m_command = nullptr;
    return nullptr;
  }

  // Find the end of the current token
  char *end = start;
  while (*end != ' ' && *end != '\0') end++;

  // If we found a space, null-terminate the token and update buffer
  if (*end == ' ') {
    *end = '\0';           // Null-terminate the current token
    *m_command = end + 1;  // Point to the rest of the string
  } else {
    // No more tokens after this one
    *m_command = nullptr;
  }

  return start;
}

void Commander::setCommand(const char *buffer) {
  if (buffer != nullptr) {
    *m_command = strdup(buffer);
    ESP_LOGD(TAG, "Command set: %s", *m_command);
  } else {
    ESP_LOGW(TAG, "Attempted to set a null buffer");
  }
}

bool Commander::runCommand(uint8_t commandID, float param) {
  ESP_LOGD(TAG, "Running command ID: %d with param: %.2f", commandID, param);

  switch (commandID) {
    case CMD_UPDATE_GAIN:
      handle_update_gain(param);
      break;
    case CMD_UPDATE_FREQMHZ:
      handle_update_freqMhz(param);
      break;
    case CMD_UPDATE_SF:
      handle_update_spreadingFactor(param);
      break;
    case CMD_UPDATE_BW:
      handle_update_bandwidthKHz(param);
      break;
    case CMD_CALIBRATE_CELL:
      handle_calibrateCell(param);
      break;
    case CMD_SET_CELL_SCALE:
      handle_setCellScale(param);
      break;
    case CMD_SET_OUTPUT:
      handle_set_OUTPUT(param);
      break;
    case CMD_HARD_RESET:
#ifdef SFTU
      ESP_LOGI(TAG, "Hard reset command received, resetting system...");
      ESP.restart();  // This will reset the ESP system
#endif
    default:
      ESP_LOGW(TAG, "Unknown command ID: %d", commandID);
      return false;
  }
  return true;
}

void Commander::handle_update_gain(float gain) {
  ESP_LOGD(TAG, "Update gain command executing");

  m_loraCom->setOutGain(static_cast<int8_t>(gain));  // Set the gain in LoRaCom
}

void Commander::handle_update_freqMhz(float freqMhz) { m_loraCom->setFrequency(freqMhz); }

void Commander::handle_update_spreadingFactor(float sf) { m_loraCom->setSpreadingFactor(static_cast<uint8_t>(sf)); }

void Commander::handle_update_bandwidthKHz(float bw) { m_loraCom->setBandwidth(bw); }

#ifdef SFTU
void Commander::handle_calibrateCell(float massKg) {
  // TODO: We need to select the MUX
  ESP_LOGD(TAG, "Calibrate cell command executing");
  float averageVoltage = m_adcADS->getAverageVolt(100, ADS1X15_REG_CONFIG_MUX_DIFF_0_1);  // Read average voltage from ADC
  m_loadCellProcessing->calibrate(massKg, averageVoltage);                                // param is the object mass
}

void Commander::handle_setCellScale(float scale) { m_loadCellProcessing->setScale(scale); }

void Commander::handle_set_OUTPUT(float indexAndState) {
  // value before decimal is the index, after decimal is the state
  int outputIndex = static_cast<int>(indexAndState);

  bool ioState = (indexAndState - (float)outputIndex) > 0.05f;

  ESP_LOGI(TAG, "Setting output %d to %s", outputIndex, ioState ? "ON" : "OFF");

  m_actuation->setDigital(PCA6408A_outputPins[outputIndex], (ioState ? OUTPUT_LOW : OUTPUT_OPEN));
}

#else
void Commander::handle_calibrateCell(float massKg) { return; }
void Commander::handle_set_OUTPUT(float indexAndState) { return; }
void Commander::handle_setCellScale(float scale) { return; }

#endif