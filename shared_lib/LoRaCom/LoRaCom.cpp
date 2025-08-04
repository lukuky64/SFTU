// LoRaCom.cpp
#include "LoRaCom.hpp"

LoRaCom::LoRaCom() {
  instance = this;  // Set the static instance pointer
  ESP_LOGI(TAG, "LoRaCom constructor called");
  currentTxIndex = -1;
}

// void LoRaCom::setRxFlag() {
//   if (instance) {
//     instance->RxFlag = true;
//   }
// }

void LoRaCom::RxTxCallback(void) {
  if (instance) {
    if (instance->TxMode) {
      int state = instance->radio->finishTransmit();
      state |= instance->radio->startReceive();
      instance->TxMode = false;
      if (state == RADIOLIB_ERR_NONE) {
        // Mark messages that do not require ACK as acknowledged and move to done queue
        if (instance->currentTxIndex >= 0 && instance->currentTxIndex < MAX_QUEUE_SIZE) {
          QueuedMessage &q = instance->sendQueue[instance->currentTxIndex];
          if (!q.reqAck && !q.acknowledged && !q.failed) {
            q.acknowledged = true;
            instance->moveToDoneQueue(q);
          }
          instance->currentTxIndex = -1;
        }
      }
    } else {
      instance->RxFlag = true;
    }
  }
}

bool LoRaCom::sendMessage(const uint8_t *data, size_t len, uint32_t timeout_ms, int queueIndex) {
  if (!radioInitialised) {
    return false;
  }

  uint32_t startTick = xTaskGetTickCount();
  while (RxFlag) {
    vTaskDelay(pdMS_TO_TICKS(10));
    if ((xTaskGetTickCount() - startTick) > pdMS_TO_TICKS(timeout_ms)) {
      ESP_LOGE(TAG, "sendMessage timeout waiting for RxFlag to clear");
      return false;
    }
  }

  int state = radio->startTransmit(data, len);
  if (state == RADIOLIB_ERR_NONE) {
    instance->TxMode = true;
    instance->currentTxIndex = queueIndex;

    // Optional: cast to LoRaMessage if logging known format
    const LoRaMessage *msg = reinterpret_cast<const LoRaMessage *>(data);
    ESP_LOGD(TAG, "Transmitting LoRaMessage: type=%u seq=%u recID=%u len=%u", msg->type, msg->sequenceID, msg->receiverID, msg->length);

    while (TxMode) {
      vTaskDelay(pdMS_TO_TICKS(10));
    }
  } else {
    ESP_LOGE(TAG, "Failed to begin transmission, code: %d", state);
    return false;
  }

  return true;
}

bool LoRaCom::checkTxMode() {
  return TxMode;  // Return the current transmission mode status
}

bool LoRaCom::getMessage(LoRaMessage *msg) {
  if (RxFlag && radioInitialised) {
    return receiveMessage(msg);
  }
  return false;
}

bool LoRaCom::checkRx() { return RxFlag; }

int32_t LoRaCom::getRssi() {
  return radio->getRSSI();  // Return the last received signal strength
}

/* ================================ SETTERS ================================ */

bool LoRaCom::setOutGain(int8_t gain) {
  // value should be bewteen -9 and 22 dBm

  int state = RADIOLIB_ERR_NONE;

  if (radioType == RADIO_SX126X) {
    state |= static_cast<SX1262 *>(radio)->setOutputPower(gain);
  } else if (radioType == RADIO_SX127X) {
    state |= static_cast<SX1278 *>(radio)->setOutputPower(gain);
  }

  // int state = radio->setOutputPower(gain);
  if (state == RADIOLIB_ERR_NONE) {
    ESP_LOGI(TAG, "Gain set to %d", gain);
    return true;
  } else {
    ESP_LOGE(TAG, "Failed to set gain with code: %d", state);
    return false;
  }
}

bool LoRaCom::setFrequency(float freqMHz) {
  // Set the frequency of the radio
  int state = radio->setFrequency(freqMHz);
  if (state == RADIOLIB_ERR_NONE) {
    ESP_LOGI(TAG, "Frequency set to %.2f MHz", freqMHz);
    return true;
  } else {
    ESP_LOGE(TAG, "Failed to set frequency with code: %d", state);
    return false;
  }
}

bool LoRaCom::setSpreadingFactor(uint8_t spreadingFactor) {
  int state = RADIOLIB_ERR_UNKNOWN;
  if (!radio) {
    ESP_LOGE(TAG, "Radio pointer is null!");
    return false;
  }
  if (radioType == RADIO_SX127X) {
    state = static_cast<SX1278 *>(radio)->setSpreadingFactor(spreadingFactor);
  } else if (radioType == RADIO_SX126X) {
    state = static_cast<SX126x *>(radio)->setSpreadingFactor(spreadingFactor);
  } else {
    ESP_LOGE(TAG, "Unknown or unsupported radio type!");
    return false;
  }
  if (state == RADIOLIB_ERR_NONE) {
    ESP_LOGI(TAG, "Spreading factor set to %d", spreadingFactor);
    return true;
  } else {
    ESP_LOGE(TAG, "Failed to set spreading factor with code: %d", state);
    return false;
  }
}

bool LoRaCom::setBandwidth(float bandwidth) {
  int state = RADIOLIB_ERR_UNKNOWN;
  if (!radio) {
    ESP_LOGE(TAG, "Radio pointer is null!");
    return false;
  }
  if (radioType == RADIO_SX127X) {
    state = static_cast<SX1278 *>(radio)->setBandwidth(bandwidth);
  } else if (radioType == RADIO_SX126X) {
    state = static_cast<SX126x *>(radio)->setBandwidth(bandwidth);
  } else {
    ESP_LOGE(TAG, "Unknown or unsupported radio type!");
    return false;
  }
  if (state == RADIOLIB_ERR_NONE) {
    ESP_LOGI(TAG, "Bandwidth set to %.2f kHz", bandwidth);
    return true;
  } else {
    ESP_LOGE(TAG, "Failed to set bandwidth with code: %d", state);
    return false;
  }
}

bool LoRaCom::enqueueMessage(LoRaMessage &msg, bool requireAck) {
  if (sendCount == MAX_QUEUE_SIZE) ESP_LOGE(TAG, "Send queue is full, cannot enqueue message");

  ESP_LOGD(TAG, "Enqueuing message with length: %u", msg.length);

  if (msg.length > MAX_PAYLOAD_SIZE) return false;
  msg.sequenceID = nextSequenceID++;
  ESP_LOGD(TAG, "Enqueuing message with sequence ID: %u", msg.sequenceID);
  sendQueue[sendTail] = {msg, 0, millis(), false, false, requireAck};
  sendTail = (sendTail + 1) % MAX_QUEUE_SIZE;
  sendCount++;
  ESP_LOGD(TAG, "Current send count: %u", sendCount);
  return true;
}

void LoRaCom::moveToDoneQueue(const QueuedMessage &q) {
  doneQueue[doneTail] = q;
  doneTail = (doneTail + 1) % MAX_QUEUE_SIZE;
  if (doneCount < MAX_QUEUE_SIZE)
    doneCount++;
  else
    doneHead = (doneHead + 1) % MAX_QUEUE_SIZE;  // Overwrite oldest

  // Invalidate the slot in sendQueue to prevent sequenceID reuse
  for (uint8_t i = 0; i < MAX_QUEUE_SIZE; i++) {
    uint8_t idx = (sendHead + i) % MAX_QUEUE_SIZE;
    if (sendQueue[idx].msg.sequenceID == q.msg.sequenceID) {
      sendQueue[idx].msg.sequenceID = 0xFF;  // Use an invalid value
      sendQueue[idx].acknowledged = true;
      sendQueue[idx].failed = false;
      break;
    }
  }
  compactSendQueue();
}

void LoRaCom::processSendQueue() {
  // Prioritise messages with reqAck == true (critical commands)
  // First, process all critical messages
  for (uint8_t i = 0; i < sendCount; i++) {
    uint8_t idx = (sendHead + i) % MAX_QUEUE_SIZE;
    QueuedMessage &q = sendQueue[idx];
    if (q.acknowledged || !q.reqAck) continue;

    if ((q.retryCount == 0) || (millis() - q.lastSendTime >= ACK_TIMEOUT_MS)) {
      if (q.retryCount < MAX_RETRIES) {
        sendMessage(reinterpret_cast<const uint8_t *>(&q.msg), sizeof(LoRaMessage), TX_TIMEOUT_MS, idx);
        q.lastSendTime = millis();
        q.retryCount++;
        ESP_LOGI(TAG, "Retrying command message (seq %u), attempt %u", q.msg.sequenceID, q.retryCount);
      } else {
        ESP_LOGE(TAG, "Max retries reached for command message (seq %u)", q.msg.sequenceID);
        q.failed = true;
        moveToDoneQueue(q);
      }
    }
  }

  // Then, process non-critical messages (reqAck == false)
  for (uint8_t i = 0; i < sendCount; i++) {
    uint8_t idx = (sendHead + i) % MAX_QUEUE_SIZE;
    QueuedMessage &q = sendQueue[idx];
    if (q.acknowledged || q.reqAck) continue;
    sendMessage(reinterpret_cast<const uint8_t *>(&q.msg), sizeof(LoRaMessage), TX_TIMEOUT_MS, idx);
    ESP_LOGI(TAG, "Transmitting message (seq %u) with no ACK required", q.msg.sequenceID);
  }
}

void LoRaCom::handleAck(uint16_t ackSeqID) {
  for (uint8_t i = 0; i < sendCount; i++) {
    uint8_t idx = (sendHead + i) % MAX_QUEUE_SIZE;
    if (sendQueue[idx].msg.sequenceID == ackSeqID) {
      sendQueue[idx].acknowledged = true;
      sendQueue[idx].failed = false;
      moveToDoneQueue(sendQueue[idx]);
      ESP_LOGI(TAG, "ACK received for message with sequence ID: %u", ackSeqID);
      break;
    }
  }
}

bool LoRaCom::receiveMessage(LoRaMessage *msg) {
  int state = radio->readData((uint8_t *)msg, sizeof(LoRaMessage));
  RxFlag = false;

  if (state != RADIOLIB_ERR_NONE) {
    return false;
  } else {
    if (msg->receiverID != DEVICE_ID && msg->receiverID != BROADCAST_ID) return false;

    switch (msg->type) {
      case TYPE_COMMAND:
        // parse and respond
        sendAck(msg->senderID, msg->sequenceID);
        break;

      case TYPE_ACK:
        if (msg->length == sizeof(AckPayload)) {
          AckPayload ack;
          memcpy(&ack, msg->payload, sizeof(ack));
          handleAck(ack.acknowledgedSequenceID);
        }
        break;
    }

    return true;
  }
}

void LoRaCom::sendAck(uint8_t targetID, uint8_t seqID) {
  AckPayload ack = {.acknowledgedSequenceID = seqID};

  LoRaMessage msg;
  msg.senderID = DEVICE_ID;
  msg.receiverID = targetID;
  msg.sequenceID = nextSequenceID++;
  msg.type = TYPE_ACK;
  msg.length = sizeof(ack);
  memcpy(msg.payload, &ack, sizeof(ack));

  sendMessage(reinterpret_cast<const uint8_t *>(&msg), sizeof(LoRaMessage), TX_TIMEOUT_MS, -1);

  ESP_LOGI(TAG, "Sent ACK for sequence ID: %u to target ID: %u", seqID, targetID);
}

void LoRaCom::compactSendQueue() {
  while (sendCount > 0 && (sendQueue[sendHead].acknowledged || sendQueue[sendHead].failed)) {
    sendHead = (sendHead + 1) % MAX_QUEUE_SIZE;
    sendCount--;
  }
}

bool LoRaCom::isAcked(uint8_t seqID) {
  for (uint8_t i = 0; i < doneCount; i++) {
    uint8_t idx = (doneHead + i) % MAX_QUEUE_SIZE;
    if (doneQueue[idx].msg.sequenceID == seqID) {
      // ESP_LOGI(TAG, "Found in done queue");
      return doneQueue[idx].acknowledged;
    }
  }
  return false;
}

bool LoRaCom::isFailed(uint8_t seqID) {
  for (uint8_t i = 0; i < doneCount; i++) {
    uint8_t idx = (doneHead + i) % MAX_QUEUE_SIZE;
    if (doneQueue[idx].msg.sequenceID == seqID) return doneQueue[idx].failed;
  }
  return false;
}

bool LoRaCom::isQueued(uint8_t seqID) {
  for (uint8_t i = 0; i < sendCount; i++) {
    uint8_t idx = (sendHead + i) % MAX_QUEUE_SIZE;
    // ESP_LOGD(TAG, "Checking sendQueue[%u] seqID: %u", idx, sendQueue[idx].msg.sequenceID);
    if (sendQueue[idx].msg.sequenceID == seqID) return true;
  }
  return false;
}

bool LoRaCom::stringToCommandPayload(CommandPayload &payload, const char *buffer) {
  // Validate input
  if (buffer == nullptr || *buffer == '\0') return false;  // Null or empty input

  // Parse the commandID
  char *endPtr;
  payload.commandID = strtoul(buffer, &endPtr, 10);
  if (buffer == endPtr || *endPtr != ' ') return false;  // Invalid format or no space after commandID

  // Now, try to parse the param as a float first
  const char *paramStart = endPtr + 1;
  char *floatEndPtr;
  float f = strtof(paramStart, &floatEndPtr);
  if (floatEndPtr != paramStart && *floatEndPtr == '\0') {
    // Successfully parsed as float
    payload.paramType = 0;  // float
    payload.paramFloat = f;
    ESP_LOGD(TAG, "Parsed CommandPayload: commandID=%u, paramType=float, param=%.2f", payload.commandID, payload.paramFloat);
    return true;
  } else {
    // Treat as string (copy up to max size)
    payload.paramType = 1;  // string
    strncpy(payload.paramString, paramStart, sizeof(payload.paramString) - 1);
    payload.paramString[sizeof(payload.paramString) - 1] = '\0';
    ESP_LOGD(TAG, "Parsed CommandPayload: commandID=%u, paramType=string, param='%s'", payload.commandID, payload.paramString);
    return true;
  }
}