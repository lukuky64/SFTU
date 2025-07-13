#include "control.hpp"

Control::Control()
{
  m_serialCom = new SerialCom();                       // Initialize SerialCom instance
  m_LoRaCom = new LoRaCom();                           // Initialize LoRaCom instance
  m_commander = new Commander(m_serialCom, m_LoRaCom); // Initialize Commander instance

  m_saveFlash = new SaveFlash(m_serialCom); // Initialize SaveFlash instance
}

void Control::setup()
{
  m_serialCom->init(115200); // Initialize serial communication

  m_LoRaCom->setRadioType(RADIO_SX126X);
  bool loraSuccess = m_LoRaCom->begin<SX1262>(SPI_CLK_RF, SPI_MISO_RF, SPI_MOSI_RF, SPI_CS_RF, RF_DIO, RF_RST, 22, RF_BUSY);

  if (!loraSuccess)
  {
    ESP_LOGE(TAG, "LoRa initialization FAILED! Check your hardware connections.");
    ESP_LOGE(TAG,
             "Pin assignments: CLK=%d, MISO=%d, MOSI=%d, CS=%d, INT=%d, "
             "RST=%d, BUSY=%d",
             SPI_CLK_RF, SPI_MISO_RF, SPI_MOSI_RF, SPI_CS_RF, RF_DIO, RF_RST, RF_BUSY);
  }
  else
  {
    ESP_LOGI(TAG, "LoRa initialized successfully!");
  }

  m_saveFlash->begin(); // Initialize flash storage

  ESP_LOGI(TAG, "Control setup complete");
}

void Control::begin()
{
  // Begin method implementation
  ESP_LOGI(TAG, "Control beginning...");

  // Delete the previous taska if they exist
  if (SerialTaskHandle != nullptr)
  {
    vTaskDelete(SerialTaskHandle);
  }

  if (LoRaTaskHandle != nullptr)
  {
    vTaskDelete(LoRaTaskHandle);
  }

  if (StatusTaskHandle != nullptr)
  {
    vTaskDelete(StatusTaskHandle);
  }

  // Create new tasks for serial data handling, LoRa data handling, and status
  // Higher priority = higher number, priorities should be 1-3 for user tasks
  xTaskCreate([](void *param)
              { static_cast<Control *>(param)->serialDataTask(); }, "SerialDataTask", 8192, this, 2, &SerialTaskHandle);

  xTaskCreate([](void *param)
              { static_cast<Control *>(param)->loRaDataTask(); }, "LoRaDataTask", 8192, this, 2, &LoRaTaskHandle);

  xTaskCreate([](void *param)
              { static_cast<Control *>(param)->statusTask(); }, "StatusTask", 8192, this, 1, &StatusTaskHandle);

  xTaskCreate([](void *param)
              { static_cast<Control *>(param)->heartBeatTask(); }, "HeartBeatTask", 2048, this, 1, &heartBeatTaskHandle);

  ESP_LOGI(TAG, "Control begun!\n");

  ESP_LOGI(TAG, "Type <help> for a list of commands");
}

void Control::heartBeatTask()
{
  pinMode(INDICATOR_LED1, OUTPUT); // Set LED pin as output
  while (true)
  {
    digitalWrite(INDICATOR_LED1, !digitalRead(INDICATOR_LED1));
    // ESP_LOGD(TAG, "LED toggled");
    vTaskDelay(pdMS_TO_TICKS(heartBeat_Interval));
  }
}

void Control::serialDataTask()
{
  char buffer[128]; // Buffer to store incoming data
  int rxIndex = 0;  // Index to track the length of the received message

  while (true)
  {
    LoRaMessage msg;
    msg.senderID = DEVICE_ID;
    msg.receiverID = BROADCAST_ID; // Broadcast to all devices
    msg.type = TYPE_COMMAND;
    msg.length = sizeof(CommandPayload);

    // Check for incoming data from the serial interface
    if (m_serialCom->getData(buffer, sizeof(buffer), &rxIndex))
    {
      // anything coming into serial should be a command (no reason to send data)

      ESP_LOGI(TAG, "Received: %s", buffer); // Log the received data

      CommandPayload payload;
      if (m_LoRaCom->stringToCommandPayload(payload, buffer))
      {
        memcpy(msg.payload, &payload, sizeof(payload));

#ifdef SFTU
        bool requireAck = false; // for this device, we want to run command directly if through serial
#else
        bool requireAck = true;
#endif

        // send to LoRaCom queue
        while (!m_LoRaCom->enqueueMessage(msg, requireAck))
          vTaskDelay(pdMS_TO_TICKS(10));
        uint8_t commandSeqID = msg.sequenceID; // Save the sequenceID immediately after enqueuing

        while (m_LoRaCom->isQueued(commandSeqID))
        {
          vTaskDelay(pdMS_TO_TICKS(10));
        }

        while (!m_LoRaCom->isAcked(commandSeqID))
        {
          vTaskDelay(pdMS_TO_TICKS(10));
        }

        m_commander->runCommand(payload.commandID, payload.param);

        // // Wait for ACK for this sequenceID
        // while (m_LoRaCom->isQueued(msg.sequenceID))
        // {
        //   ESP_LOGI(TAG, "Waiting in queue");
        //   vTaskDelay(pdMS_TO_TICKS(10));
        // }

        // clear the buffer for the next message
        memset(buffer, 0, sizeof(buffer));
        rxIndex = 0; // Reset the index
      }
    }
    vTaskDelay(pdMS_TO_TICKS(serial_Interval));
  }
}

void Control::loRaDataTask()
{

  LoRaMessage msg;

  while (true)
  {
    // Check for incoming data from the LoRa interface
    if (m_LoRaCom->getMessage(&msg))
    {

      if (msg.type == TYPE_ACK)
        continue;
      else if (msg.type == TYPE_COMMAND)
      {
        CommandPayload payload;
        memcpy(&payload, msg.payload, sizeof(payload));
        m_commander->runCommand(payload.commandID, payload.param);
      }
      else if (msg.type == TYPE_STATUS)
      {
        StatusPayload payload;
        memcpy(&payload, msg.payload, sizeof(payload));
        String statusMsg = String("status ") + "ID:" + String(msg.senderID) + " RSSI:" + String(payload.rssi) + " battVoltage:" + String(payload.batteryVoltage) + " status:" + String(payload.status) + ("\n");
        m_serialCom->sendData(statusMsg.c_str());
      }
      // Clear for the next iteration
      memset(&msg, 0, sizeof(msg));
    }
    else
    {
      m_LoRaCom->processSendQueue(); // Process the send queue to send any pending messages
    }

    vTaskDelay(pdMS_TO_TICKS(lora_Interval));
  }
}

void Control::statusTask()
{
  static unsigned long lastStatusTime = 0;

  LoRaMessage msg;
  msg.senderID = DEVICE_ID;
  msg.receiverID = BROADCAST_ID;
  msg.type = TYPE_STATUS;
  msg.length = sizeof(StatusPayload);

  while (true)
  {
    StatusPayload payload;
    payload.rssi = static_cast<int8_t>(m_LoRaCom->getRssi()); // value from -128 to 127, this should be fine
    payload.batteryVoltage = m_batteryLevel;
    payload.status = deviceStatus::STATUS_OK;
    memcpy(msg.payload, &payload, sizeof(payload));

    String statusMsg = String("status ") + "ID:" + String(msg.senderID) + " RSSI:" + String(payload.rssi) + " battVoltage:" + String(payload.batteryVoltage) + " status:" + String(payload.status) + ("\n");

    // Send over serial first (this should be fast)
    m_serialCom->sendData(statusMsg.c_str());

    // Try LoRa transmission with timeout protection

    if (m_LoRaCom->enqueueMessage(msg, false))
      ESP_LOGD(TAG, "Adding to transmit queue...");

    // checkTaskStack();

    vTaskDelay(pdMS_TO_TICKS(status_Interval));
  }
}

// void Control::interpretMessage(const char *buffer, bool relayMsgLoRa)
// {
//   m_commander->setCommand(buffer); // Set the command in the commander
//   char *token = m_commander->readAndRemove();

//   // eg: "command update gain 22"
//   // eg: "status <deviceID> <RSSI> <batteryLevel> <mode> <status>"
//   // eg: "data <payload>"

//   if (c_cmp(token, "command"))
//   {
//     if (relayMsgLoRa)
//     {
//       // send to other devices to sync parameters
//       m_LoRaCom->sendMessage(buffer, 2000);
//     }
//     // should probably wait for a success reply before changing THIS device
//     ESP_LOGD(TAG, "Processing command: %s", buffer);
//     m_commander->checkCommand();
//   }
//   else if (c_cmp(token, "data"))
//   {
//     processData(buffer);
//   }
//   else if (c_cmp(token, "status"))
//   {
//     processData(buffer);
//   }
//   else if (c_cmp(token, "help"))
//   {
//     ESP_LOGI(TAG,
//              "Message format: <type> <data1> <data2> ...\n"
//              "Valid types:\n"
//              "  - flash: for flash read and save\n"
//              "  - command: for device control\n"
//              "  - data: for data transmission\n"
//              "  - message: for standard messages\n"
//              "  - flash: to print and auto erase logs\n"
//              "  - status: for device status\n"
//              "  - help: for displaying help information");
//   }
//   else if (c_cmp(token, "flash"))
//   {
//     m_saveFlash->readFile();
//     m_saveFlash->removeFile(); // Update the flash storage
//     m_saveFlash->begin();      // Reinitialize the flash storage
//   }
// }

void Control::processData(const char *buffer)
{
  // Process the data message
  ESP_LOGD(TAG, "Processing data");

  // remove the "data" prefix
  const char *dataStart = strchr(buffer, ' ') + 1; // Find the first space
  if (dataStart == nullptr)
  {
    ESP_LOGE(TAG, "Invalid data format: %s", buffer);
    return; // Invalid format, return early
  }

  m_serialCom->sendData(buffer); // Send the data part over serial
  m_serialCom->sendData("\n");

  // save dataStart to flash
  m_saveFlash->writeData((dataStart + String("\n")).c_str());

  ESP_LOGI(TAG, "Data processing complete");
}