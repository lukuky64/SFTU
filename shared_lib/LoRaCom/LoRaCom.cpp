// LoRaCom.cpp
#include "LoRaCom.hpp"

LoRaCom::LoRaCom()
{
  instance = this; // Set the static instance pointer
  ESP_LOGI(TAG, "LoRaCom constructor called");
}

// void LoRaCom::setRxFlag() {
//   if (instance) {
//     instance->RxFlag = true;
//   }
// }

void LoRaCom::RxTxCallback(void)
{
  if (instance)
  {
    if (instance->TxMode)
    {
      int state = instance->radio->finishTransmit();
      state |= instance->radio->startReceive();
      instance->TxMode = false;
      if (state == RADIOLIB_ERR_NONE)
      {
        ESP_LOGI(TAG, "Transmission finished");
      }
      else
      {
        ESP_LOGE(TAG, "Transmission failed, code: %d", state);
      }
    }
    else
    {
      instance->RxFlag = true;
    }
  }
}

void LoRaCom::sendMessage(const char *msg, uint32_t timeout_ms)
{
  if (!radioInitialised || (msg[0] == '\0'))
  {
    return;
  }
  uint32_t startTick = xTaskGetTickCount();
  while (RxFlag)
  {
    vTaskDelay(pdMS_TO_TICKS(1));
    if ((xTaskGetTickCount() - startTick) > pdMS_TO_TICKS(timeout_ms))
    {
      ESP_LOGE(TAG, "sendMessage timeout waiting for RxFlag to clear");
      return;
    }
  }

  int state = radio->startTransmit(msg);
  if (state == RADIOLIB_ERR_NONE)
  {
    instance->TxMode = true;
    ESP_LOGI(TAG, "Transmitting: <%s>", msg);
    while (TxMode)
      vTaskDelay(pdMS_TO_TICKS(1)); // Wait for transmission to finish
  }
  else
  {
    ESP_LOGE(TAG, "Failed to begin transmission, code: %d", state);
  }
}

bool LoRaCom::checkTxMode()
{
  return TxMode; // Return the current transmission mode status
}

bool LoRaCom::getMessage(char *buffer, size_t len)
{
  if (RxFlag && radioInitialised)
  {
    int state = radio->readData(reinterpret_cast<uint8_t *>(buffer), len);
    RxFlag = false;
    state |= radio->startReceive();
    return (state == RADIOLIB_ERR_NONE);
  }
  return false;
}

bool LoRaCom::checkRx() { return RxFlag; }

int32_t LoRaCom::getRssi()
{
  return radio->getRSSI(); // Return the last received signal strength
}

/* ================================ SETTERS ================================ */

bool LoRaCom::setOutGain(int8_t gain)
{
  // value should be bewteen -9 and 22 dBm

  int state = RADIOLIB_ERR_NONE;

  if (radioType == RADIO_SX126X)
  {
    state |= static_cast<SX1262 *>(radio)->setOutputPower(gain);
  }
  else if (radioType == RADIO_SX127X)
  {
    state |= static_cast<SX1278 *>(radio)->setOutputPower(gain);
  }

  // int state = radio->setOutputPower(gain);
  if (state == RADIOLIB_ERR_NONE)
  {
    ESP_LOGI(TAG, "Gain set to %d", gain);
    return true;
  }
  else
  {
    ESP_LOGE(TAG, "Failed to set gain with code: %d", state);
    return false;
  }
}

bool LoRaCom::setFrequency(float freqMHz)
{
  // Set the frequency of the radio
  int state = radio->setFrequency(freqMHz);
  if (state == RADIOLIB_ERR_NONE)
  {
    ESP_LOGI(TAG, "Frequency set to %.2f MHz", freqMHz);
    return true;
  }
  else
  {
    ESP_LOGE(TAG, "Failed to set frequency with code: %d", state);
    return false;
  }
}

bool LoRaCom::setSpreadingFactor(uint8_t spreadingFactor)
{
  int state = RADIOLIB_ERR_UNKNOWN;
  if (!radio)
  {
    ESP_LOGE(TAG, "Radio pointer is null!");
    return false;
  }
  if (radioType == RADIO_SX127X)
  {
    state = static_cast<SX1278 *>(radio)->setSpreadingFactor(spreadingFactor);
  }
  else if (radioType == RADIO_SX126X)
  {
    state = static_cast<SX126x *>(radio)->setSpreadingFactor(spreadingFactor);
  }
  else
  {
    ESP_LOGE(TAG, "Unknown or unsupported radio type!");
    return false;
  }
  if (state == RADIOLIB_ERR_NONE)
  {
    ESP_LOGI(TAG, "Spreading factor set to %d", spreadingFactor);
    return true;
  }
  else
  {
    ESP_LOGE(TAG, "Failed to set spreading factor with code: %d", state);
    return false;
  }
}

bool LoRaCom::setBandwidth(float bandwidth)
{
  int state = RADIOLIB_ERR_UNKNOWN;
  if (!radio)
  {
    ESP_LOGE(TAG, "Radio pointer is null!");
    return false;
  }
  if (radioType == RADIO_SX127X)
  {
    state = static_cast<SX1278 *>(radio)->setBandwidth(bandwidth);
  }
  else if (radioType == RADIO_SX126X)
  {
    state = static_cast<SX126x *>(radio)->setBandwidth(bandwidth);
  }
  else
  {
    ESP_LOGE(TAG, "Unknown or unsupported radio type!");
    return false;
  }
  if (state == RADIOLIB_ERR_NONE)
  {
    ESP_LOGI(TAG, "Bandwidth set to %.2f kHz", bandwidth);
    return true;
  }
  else
  {
    ESP_LOGE(TAG, "Failed to set bandwidth with code: %d", state);
    return false;
  }
}

// bool LoRaCom::enqueueMessage(LoRaMessage &msg)
// {
//   if (queueCount >= MAX_QUEUE_SIZE)
//     return false;

//   msg.sequenceID = nextSequenceID++;
//   sendQueue[queueCount++] = {
//       .msg = msg,
//       .retryCount = 0,
//       .lastSendTime = millis(),
//       .acknowledged = false};
//   return true;
// }

// void LoRaCom::processSendQueue()
// {
//   for (uint8_t i = 0; i < queueCount; i++)
//   {
//     QueuedMessage &q = sendQueue[i];
//     if (q.acknowledged)
//       continue;

//     if (millis() - q.lastSendTime >= ACK_TIMEOUT_MS)
//     {
//       if (q.retryCount < MAX_RETRIES)
//       {
//         LoRa.beginPacket();
//         LoRa.write((uint8_t *)&q.msg, sizeof(LoRaMessage));
//         LoRa.endPacket();

//         q.lastSendTime = millis();
//         q.retryCount++;
//         ESP_LOGI(TAG, "Retrying message (seq %u), attempt %u", q.msg.sequenceID, q.retryCount);
//       }
//       else
//       {
//         ESP_LOGE(TAG, "Max retries reached for message (seq %u)", q.msg.sequenceID);
//         q.acknowledged = true;
//       }
//     }
//   }
// }

// void LoRaCom::handleAck(uint16_t ackSeqID)
// {
//   for (uint8_t i = 0; i < queueCount; i++)
//   {
//     if (sendQueue[i].msg.sequenceID == ackSeqID)
//     {
//       sendQueue[i].acknowledged = true;
//       ESP_LOGI(TAG, "ACK received for message with sequence ID: %u", ackSeqID);
//       break;
//     }
//   }
//   compactSendQueue();
// }

// void LoRaCom::receiveMessage()
// {
//   if (LoRa.parsePacket() == sizeof(LoRaMessage))
//   {
//     LoRaMessage msg;
//     LoRa.readBytes((uint8_t *)&msg, sizeof(msg));

//     if (msg.receiverID != DEVICE_ID && msg.receiverID != BROADCAST_ID)
//       return;

//     switch (msg.type)
//     {
//     case TYPE_DATA:
//       // parse and respond
//       sendAck(msg.senderID, msg.sequenceID);
//       break;

//     case TYPE_ACK:
//       if (msg.length == sizeof(AckPayload))
//       {
//         AckPayload ack;
//         memcpy(&ack, msg.payload, sizeof(ack));
//         handleAck(ack.acknowledgedSequenceID);
//       }
//       break;
//     }
//   }
// }

// void LoRaCom::sendAck(uint8_t targetID, uint16_t seqID)
// {
//   AckPayload ack = {.acknowledgedSequenceID = seqID};

//   LoRaMessage msg;
//   msg.senderID = DEVICE_ID;
//   msg.receiverID = targetID;
//   msg.sequenceID = nextSequenceID++;
//   msg.type = TYPE_ACK;
//   msg.length = sizeof(ack);
//   memcpy(msg.payload, &ack, sizeof(ack));

//   LoRa.beginPacket();
//   LoRa.write((uint8_t *)&msg, sizeof(msg));
//   LoRa.endPacket();

//   Serial.printf("Sent ACK for seq %u\n", seqID);
// }

// void LoRaCom::compactSendQueue()
// {
//   uint8_t i = 0;
//   while (i < queueCount)
//   {
//     if (sendQueue[i].acknowledged)
//     {
//       // Shift all remaining messages down by one
//       for (uint8_t j = i; j < queueCount - 1; j++)
//       {
//         sendQueue[j] = sendQueue[j + 1];
//       }
//       queueCount--; // Reduce queue size
//       // Do not increment i → check the new message that was shifted into position i
//     }
//     else
//     {
//       i++;
//     }
//   }
// }
