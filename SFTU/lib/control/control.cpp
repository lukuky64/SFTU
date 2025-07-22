
#include "control.hpp"

Control::Control() {
  m_ANALOG_I2C_BUS = new TwoWire(0);
  m_I2C_BUS = new TwoWire(1);
  m_SPI_BUS = new SPIClass();

  m_serialCom = new SerialCom();
  m_LoRaCom = new LoRaCom();

  m_adcADS_12 = new adcADS(*m_ANALOG_I2C_BUS);
  m_adcADS_34 = new adcADS(*m_ANALOG_I2C_BUS);

  m_adcProcessing = new ADCprocessing(CELL_SCALE);

  m_sdTalker = new SD_Talker();

  m_battMonitor = new BattMonitor(VOLTAGE_SENSE, VBATT_SCALE);

#ifdef SFTU
  m_actuation = new Actuation(PCA6408A_SLAVE_ADDRESS_L, PCA6408A_SLAVE_ADDRESS_H, *m_I2C_BUS);
  m_commander = new Commander(m_serialCom, m_LoRaCom, m_actuation, m_adcADS_12, m_adcProcessing);  // TODO: Probably want control of both ADCs in Commander

  m_display = new Display();
#else
  m_commander = new Commander(m_serialCom, m_LoRaCom);
  m_saveFlash = new SaveFlash(m_serialCom);
#endif
}

void Control::setup() {
#ifdef SFTU
  m_I2C_BUS->setClock(400'000);
  m_ANALOG_I2C_BUS->setClock(400'000);
  m_SPI_BUS->setFrequency(40'000'000);

  m_I2C_BUS->begin(I2C2_SDA, I2C2_SCL);
  m_ANALOG_I2C_BUS->begin(I2C1_SDA, I2C1_SCL);
  m_SPI_BUS->begin(SPI_CLK_SD, SPI_MISO_SD, SPI_MOSI_SD);

  m_actuation->init();
  m_sdTalker->begin(SD_CD, SPI_CS_SD, *m_SPI_BUS);  // Initialize SD card
  m_adcADS_12->init(ADS0_ADDR);                     // Use ADS0 address
  m_adcADS_34->init(ADS1_ADDR);                     // Use ADS1 address
  m_battMonitor->init();                            // Initialize battery monitor

  m_display->init(*m_I2C_BUS);  // Initialize the display

  // from testing, queue goes up to ~200 samples during sd write
  m_adcQueue = xQueueCreate(512, sizeof(SampleWithTimestamp));

#else
#endif

  m_serialCom->init(115200);  // Initialize serial communication

  m_LoRaCom->setRadioType(RADIO_SX127X);
  bool loraSuccess = m_LoRaCom->begin<SX1276>(SPI_CLK_RF, SPI_MISO_RF, SPI_MOSI_RF, SPI_CS_RF, RF_DIO, RF_RST, 20);

  if (!loraSuccess) {
    ESP_LOGE(TAG, "LoRa initialization FAILED! Check your hardware connections.");
    ESP_LOGE(TAG,
             "Pin assignments: CLK=%d, MISO=%d, MOSI=%d, CS=%d, INT=%d, "
             "RST=%d",
             SPI_CLK_RF, SPI_MISO_RF, SPI_MOSI_RF, SPI_CS_RF, RF_DIO, RF_RST);
  }

#ifdef SFTU
  // nothing
#else
  m_saveFlash->begin();  // Initialize flash storage
#endif

  ESP_LOGI(TAG, "Control setup complete");
}

void Control::begin() {
  // Begin method implementation
  ESP_LOGI(TAG, "Control beginning...");

  // Delete the previous tasks if they exist
  if (m_taskHandles.SerialTaskHandle != nullptr) {
    vTaskDelete(m_taskHandles.SerialTaskHandle);
  }

  if (m_taskHandles.LoRaTaskHandle != nullptr) {
    vTaskDelete(m_taskHandles.LoRaTaskHandle);
  }

  if (m_taskHandles.StatusTaskHandle != nullptr) {
    vTaskDelete(m_taskHandles.StatusTaskHandle);
  }

  if (m_taskHandles.heartBeatTaskHandle != nullptr) {
    vTaskDelete(m_taskHandles.heartBeatTaskHandle);
  }

  if (m_taskHandles.analogTaskHandle != nullptr) {
    vTaskDelete(m_taskHandles.analogTaskHandle);
  }

  if (m_taskHandles.sdTaskHandle != nullptr) {
    vTaskDelete(m_taskHandles.sdTaskHandle);
  }

  if (m_taskHandles.displayTaskHandle != nullptr) {
    vTaskDelete(m_taskHandles.displayTaskHandle);
  }

  // Create new tasks for serial data handling, LoRa data handling, and status
  // Higher priority = higher number, priorities should be 1-3 for user tasks
  xTaskCreate([](void *param) { static_cast<Control *>(param)->serialDataTask(); }, "SerialDataTask", 8192, this, 2, &m_taskHandles.SerialTaskHandle);

  xTaskCreate([](void *param) { static_cast<Control *>(param)->loRaDataTask(); }, "LoRaDataTask", 8192, this, 2, &m_taskHandles.LoRaTaskHandle);

  xTaskCreate([](void *param) { static_cast<Control *>(param)->statusTask(); }, "StatusTask", 8192, this, 1, &m_taskHandles.StatusTaskHandle);

  xTaskCreate([](void *param) { static_cast<Control *>(param)->heartBeatTask(); }, "HeartBeatTask", 4096, this, 1, &m_taskHandles.heartBeatTaskHandle);

  xTaskCreate([](void *param) { static_cast<Control *>(param)->analogTask(); }, "AnalogTask", 8192, this, 3, &m_taskHandles.analogTaskHandle);

  xTaskCreate([](void *param) { static_cast<Control *>(param)->sdTask(); }, "sdTask", 8192, this, 3, &m_taskHandles.sdTaskHandle);

  xTaskCreate([](void *param) { static_cast<Control *>(param)->displayTask(); }, "displayTask", 4096, this, 1, &m_taskHandles.displayTaskHandle);

  ESP_LOGI(TAG, "Control begun!\n");

  ESP_LOGI(TAG, "Type <help> for a list of commands");
}

void Control::heartBeatTask() {
  pinMode(INDICATOR_LED1, OUTPUT);  // Set LED pin as output
  while (true) {
    digitalWrite(INDICATOR_LED1, !digitalRead(INDICATOR_LED1));
    // ESP_LOGD(TAG, "LED toggled");
    vTaskDelay(pdMS_TO_TICKS(heartBeat_Interval));
  }
}

void Control::displayTask() {
  // m_display->dim(true);
  m_display->begin();  // Begin the display

  while (true) {
    // Update the display with the current force value
    if (m_adcQueue != nullptr) {
      SampleWithTimestamp sample;
      if (xQueuePeek(m_adcQueue, &sample, pdMS_TO_TICKS(10))) {
        m_display->drawForce(sample.value1, true);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(40));
  }
}

void Control::analogTask() {
  m_adcADS_12->setInputConfig(GAIN_FOUR, RATE_ADS1115_860SPS, ADS1X15_REG_CONFIG_MUX_DIFF_0_1);  // ADS1X15_REG_CONFIG_MUX_DIFF_0_1, ADS1X15_REG_CONFIG_MUX_DIFF_2_3
  m_adcADS_12->setDataRate(ADC_SPS);
  // m_adcADS_12->startContinuous();

  m_adcADS_34->setInputConfig(GAIN_FOUR, RATE_ADS1115_860SPS, ADS1X15_REG_CONFIG_MUX_DIFF_2_3);  // ADS1X15_REG_CONFIG_MUX_DIFF_0_1, ADS1X15_REG_CONFIG_MUX_DIFF_2_3
  m_adcADS_34->setDataRate(ADC_SPS);
  // m_adcADS_34->startContinuous();

  uint64_t interval_us = (uint64_t)(1e6 / (double)ADC_SPS);
  TickType_t interval_ticks = pdMS_TO_TICKS((uint32_t)(interval_us / 1000));

  vTaskDelay(pdMS_TO_TICKS(100));
  m_adcADS_12->getAverageVolt(100, ADS1X15_REG_CONFIG_MUX_DIFF_0_1);  // throw away some data first
  float averageSample = m_adcADS_12->getAverageVolt(200, ADS1X15_REG_CONFIG_MUX_DIFF_0_1);
  m_adcProcessing->tareVolts(averageSample);

  uint64_t lastMicros = 0;
  while (true) {
    lastMicros = micros();
    queueSample();

    // vTaskDelay(pdMS_TO_TICKS(1));  // Yield to other tasks

    vTaskDelay(pdMS_TO_TICKS(interval_ticks));  // Yield to other tasks

    // if ((micros() - lastMicros) < interval_us) {
    // };

    while ((micros() - lastMicros) < interval_us);  // this will block
  }
}

void Control::queueSample() {
  SampleWithTimestamp sample;
  static uint64_t startMicros = micros();
  sample.timestamp = micros() - startMicros;

  sample.value1 = m_adcProcessing->processVtoN(m_adcADS_12->readNewVolt(ADS1X15_REG_CONFIG_MUX_DIFF_0_1));
  sample.value2 = m_adcProcessing->processVtoN(m_adcADS_12->readNewVolt(ADS1X15_REG_CONFIG_MUX_DIFF_2_3));

  sample.value3 = m_adcProcessing->processVtoN(m_adcADS_34->readNewVolt(ADS1X15_REG_CONFIG_MUX_DIFF_0_1));
  sample.value4 = m_adcProcessing->processVtoN(m_adcADS_34->readNewVolt(ADS1X15_REG_CONFIG_MUX_DIFF_2_3));

  if (xQueueSend(m_adcQueue, &sample, 0) != pdPASS) {
    // Queue is full, remove oldest and try again
    SampleWithTimestamp dummy;
    xQueueReceive(m_adcQueue, &dummy, 0);
    xQueueSend(m_adcQueue, &sample, 0);
  }
}

void Control::sdTask() {
  pinMode(INDICATOR_LED3, OUTPUT);
  constexpr size_t blockSize = 512;
  static SampleWithTimestamp block[blockSize];

  size_t count = 0;

  // Max wait before flushing
  const TickType_t blockTimeout = pdMS_TO_TICKS(1000);

  TickType_t lastBlockTime = xTaskGetTickCount();

  while (true) {
    while (!m_sdTalker->checkFileOpen()) {
      m_sdTalker->startNewLog("/log");
      vTaskDelay(pdMS_TO_TICKS(500));
    }

    // print size of queue
    // ESP_LOGD(TAG, "Queue size: %d", uxQueueMessagesWaiting(m_adcQueue));

    if (xQueueReceive(m_adcQueue, &block[count], blockTimeout)) {
      count++;
      // Try to fill the block as much as possible, but don't block
      while (count < blockSize && xQueueReceive(m_adcQueue, &block[count], 0)) {
        count++;
      }
    }

    // Write if block is full or timeout has passed and we have data
    TickType_t now = xTaskGetTickCount();
    if (count >= blockSize || (count > 0 && (now - lastBlockTime) >= blockTimeout)) {
      // Write both value and timestamp to SD (update SD_Talker as needed)
      bool blockWritten = m_sdTalker->writeBlockToSD(block, count);
      if (blockWritten) {
        // ESP_LOGI(TAG, "Wrote %zu samples to SD", count);
        digitalWrite(INDICATOR_LED3, !digitalRead(INDICATOR_LED3));
      } else {
        digitalWrite(INDICATOR_LED3, LOW);
      }
      count = 0;
      lastBlockTime = now;
    }
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}

void Control::serialDataTask() {
  char buffer[128];  // Buffer to store incoming data
  int rxIndex = 0;   // Index to track the length of the received message

  // esp_task_wdt_config_t config = {
  //     .timeout_ms = 1000, // Set the watchdog timeout to 1 second
  //     .trigger_panic = true, // Trigger a panic if the watchdog is not reset
  // };

  // esp_task_wdt_init(&config);
  // esp_task_wdt_add(NULL);      // Add current task

  while (true) {
    LoRaMessage msg;
    msg.senderID = DEVICE_ID;
    msg.receiverID = BROADCAST_ID;  // Broadcast to all devices
    msg.type = TYPE_COMMAND;
    msg.length = sizeof(CommandPayload);

    // Check for incoming data from the serial interface
    if (m_serialCom->getData(buffer, sizeof(buffer), &rxIndex)) {
      // anything coming into serial should be a command (no reason to send data)

      ESP_LOGI(TAG, "Received: %s", buffer);  // Log the received data

      CommandPayload payload;
      if (m_LoRaCom->stringToCommandPayload(payload, buffer)) {
        memcpy(msg.payload, &payload, sizeof(payload));

#ifdef SFTU
        bool requireAck = false;  // for this device, we want to run command directly if through serial
#else
        bool requireAck = true;
#endif

        // send to LoRaCom queue
        while (!m_LoRaCom->enqueueMessage(msg, requireAck)) vTaskDelay(pdMS_TO_TICKS(10));
        uint8_t commandSeqID = msg.sequenceID;  // Save the sequenceID immediately after enqueuing

        while (m_LoRaCom->isQueued(commandSeqID)) {
          vTaskDelay(pdMS_TO_TICKS(10));
        }

        while (!m_LoRaCom->isAcked(commandSeqID)) {
          vTaskDelay(pdMS_TO_TICKS(10));
        }

        ESP_LOGI(TAG, "LINE 300");

        m_commander->runCommand(payload.commandID, payload.param);
        // // Wait for ACK for this sequenceID
        // while (m_LoRaCom->isQueued(msg.sequenceID))
        // {
        //   ESP_LOGI(TAG, "Waiting in queue");
        //   vTaskDelay(pdMS_TO_TICKS(10));
        // }

        // clear the buffer for the next message
        memset(buffer, 0, sizeof(buffer));
        rxIndex = 0;  // Reset the index
      }
    }
    vTaskDelay(pdMS_TO_TICKS(serial_Interval));
    // esp_task_wdt_reset();
  }
}

void Control::loRaDataTask() {
  pinMode(INDICATOR_LED2, OUTPUT);  // Set LED pin as output

  LoRaMessage msg;

  while (true) {
    // Check for incoming data from the LoRa interface
    if (m_LoRaCom->getMessage(&msg)) {
      digitalWrite(INDICATOR_LED2, !digitalRead(INDICATOR_LED2));  // Toggle LED

      if (msg.type == TYPE_ACK)
        continue;
      else if (msg.type == TYPE_COMMAND) {
        CommandPayload payload;
        memcpy(&payload, msg.payload, sizeof(payload));
        m_commander->runCommand(payload.commandID, payload.param);
      } else if (msg.type == TYPE_STATUS) {
        StatusPayload payload;
        memcpy(&payload, msg.payload, sizeof(payload));
        String statusMsg = String("status ") + "ID:" + String(msg.senderID) + " RSSI:" + String(payload.rssi) + " battVoltage:" + String(payload.batteryVoltage) + " status:" + String(payload.status) + ("\n");
        m_serialCom->sendData(statusMsg.c_str());
      }
      // Clear for the next iteration
      memset(&msg, 0, sizeof(msg));
    } else {
      m_LoRaCom->processSendQueue();  // Process the send queue to send any pending messages
    }

    vTaskDelay(pdMS_TO_TICKS(lora_Interval));
  }
}

void Control::statusTask() {
  static unsigned long lastStatusTime = 0;

  LoRaMessage msg;
  msg.senderID = DEVICE_ID;
  msg.receiverID = BROADCAST_ID;
  msg.type = TYPE_STATUS;
  msg.length = sizeof(StatusPayload);

  while (true) {
    StatusPayload payload;
    payload.rssi = static_cast<int8_t>(m_LoRaCom->getRssi());  // value from -128 to 127, this should be fine
    payload.batteryVoltage = m_battMonitor->getScaledVoltage(/*num_readings*/ 20);
    payload.status = deviceStatus::STATUS_OK;
    memcpy(msg.payload, &payload, sizeof(payload));

    String statusMsg = String("status ") + "ID:" + String(msg.senderID) + " RSSI:" + String(payload.rssi) + " battVoltage:" + String(payload.batteryVoltage) + " status:" + String(payload.status) + ("\n");

    // Send over serial first (this should be fast)
    m_serialCom->sendData(statusMsg.c_str());

    // Try LoRa transmission with timeout protection

    if (m_LoRaCom->enqueueMessage(msg, false)) ESP_LOGD(TAG, "Adding to transmit queue...");

    // checkTaskStack();

    vTaskDelay(pdMS_TO_TICKS(status_Interval));
  }
}

// void Control::interpretMessage(const char *buffer, bool relayMsgLoRa)
// {
//   m_commander->setCommand(buffer); // Set the command in the commander
//   char *token = m_commander->readAndRemove();

//   if (c_cmp(token, "command"))
//   {
//     if (relayMsgLoRa)
//     {
//       // send to other devices to sync parameters
//       while (m_LoRaCom->checkRx())
//         vTaskDelay(pdMS_TO_TICKS(1));
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
//              "  - command: for device control\n"
//              "  - data: for data transmission\n"
//              "  - message: for standard messages\n"
//              "  - flash: to print and auto erase logs\n"
//              "  - status: for device status\n"
//              "  - help: for displaying help information");
//   }

// #ifdef SFTU
// #else
//   else if (c_cmp(token, "flash"))
//   {
//     m_saveFlash->readFile();
//     m_saveFlash->removeFile(); // Update the flash storage
//     m_saveFlash->begin();      // Reinitialize the flash storage
//   }
// #endif
// }

void Control::processData(const char *buffer) {
  ESP_LOGD(TAG, "Processing data");

  m_serialCom->sendData(buffer);  // Send the data part over serial
  m_serialCom->sendData("\n");

#ifdef SFTU
#else
  // save dataStart to flash
  m_saveFlash->writeData((dataStart + String("\n")).c_str());
#endif
}

void Control::checkTaskStack() {
  // print how much stack is left on each task
  for (const auto &task : m_taskHandleMap) {
    if (task.handle != nullptr && *task.handle != nullptr) {
      UBaseType_t stackHighWaterMark = uxTaskGetStackHighWaterMark(*task.handle);
      ESP_LOGI(TAG, "%s: Stack high water mark: %u", task.name.c_str(), stackHighWaterMark);
    } else {
      ESP_LOGI(TAG, "%s: Task handle is null", task.name.c_str());
    }
  }
}