
#include "control.hpp"

Control::Control() {
  m_ANALOG_I2C_BUS = new TwoWire(0);
  m_I2C_BUS = new TwoWire(1);
  m_SPI_BUS = new SPIClass();

  m_serialCom = new SerialCom();
  m_LoRaCom = new LoRaCom();

  m_adcADS_12 = new adcADS(*m_ANALOG_I2C_BUS);
  m_adcADS_34 = new adcADS(*m_ANALOG_I2C_BUS);

  // m_loadCell1 = new loadCellProcessing();
  // m_pressTran1 = new PTProcessing();

  m_sdTalker = new SD_Talker();

  // Example: create config object
  m_config = new ControlConfig();

  m_battMonitor = new BattMonitor(VOLTAGE_SENSE, VBATT_SCALE);

#ifdef SFTU
  m_actuation = new Actuation(PCA6408A_SLAVE_ADDRESS_L, PCA6408A_SLAVE_ADDRESS_H, *m_I2C_BUS);
  m_commander = new Commander(m_serialCom, m_LoRaCom, m_actuation, m_adcADS_12, m_adcProcessors);  // TODO: Probably want control of both ADCs in Commander

  m_display = new Display();
#else
  m_commander = new Commander(m_serialCom, m_LoRaCom);
  m_saveFlash = new SaveFlash(m_serialCom);
#endif

  m_latestSampleMutex = xSemaphoreCreateMutex();
}

void Control::setup() {
#ifdef SFTU

  m_I2C_BUS->begin(I2C2_SDA, I2C2_SCL);
  m_ANALOG_I2C_BUS->begin(I2C1_SDA, I2C1_SCL);
  m_SPI_BUS->begin(SPI_CLK_SD, SPI_MISO_SD, SPI_MOSI_SD);

  m_I2C_BUS->setClock(400'000);
  m_ANALOG_I2C_BUS->setClock(1'000'000);
  m_SPI_BUS->setFrequency(40'000'000);
  // ESP_LOGD(TAG, "Analog clock actual frequency: %d Hz", m_ANALOG_I2C_BUS->getClock());

  m_actuation->init();
  m_sdTalker->begin(SD_CD, SPI_CS_SD, *m_SPI_BUS);  // Initialize SD card

  // Load config from SD card
  if (!m_config->loadFromSD(*m_sdTalker, "/config.json")) {
    ESP_LOGW(TAG, "Failed to load config from SD, using defaults");
    // Save defaults if file doesn't exist
    m_config->saveToSD(*m_sdTalker, "/config.json");
  } else {
    ESP_LOGI(TAG, "Loaded config from SD");
  }

  m_adcADS_12->init(ADS0_ADDR);  // Use ADS0 address
  m_adcADS_34->init(ADS1_ADDR);  // Use ADS1 address
  m_battMonitor->init();         // Initialize battery monitor

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

  // Example: Access config values
  // int input0 = m_config->adc_inputs[0];
  // float scale0 = m_config->scale_factors[0];
  // uint32_t freq = m_config->rf_frequency;
  // int sps = m_config->sampling_rate;
  // int mode = m_config->mode;
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

  xTaskCreate([](void *param) { static_cast<Control *>(param)->loRaDataTask(); }, "LoRaDataTask", 8192, this, 3, &m_taskHandles.LoRaTaskHandle);

  xTaskCreate([](void *param) { static_cast<Control *>(param)->statusTask(); }, "StatusTask", 8192, this, 2, &m_taskHandles.StatusTaskHandle);

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
    // if (m_adcQueue != nullptr) {
    //   if (xQueuePeek(m_adcQueue, &sample, pdMS_TO_TICKS(50))) {
    //   }
    // }
    SampleWithTimestamp sample;
    getLatestSample(sample);  // Get the latest sample from the queue
    m_display->drawData(sample.value1, sample.value2, sample.value3, sample.value4, sample.value5, sample.value6, sample.value7, sample.value8, true);
    vTaskDelay(pdMS_TO_TICKS(40));
  }
}

void Control::analogTask() {
  m_adcADS_12->setInputConfig(GAIN_ONE, RATE_ADS1115_860SPS);
  m_adcADS_34->setInputConfig(GAIN_ONE, RATE_ADS1115_860SPS);

  uint64_t interval_us = (uint64_t)(1e6 / (double)ADC_SPS);

  // Set up load cell processing
  // float averageSample = m_adcADS_12->getAverageVolt(200, ADS1X15_REG_CONFIG_MUX_DIFF_0_1);
  // m_loadCell1->tareVolts(averageSample);
  // m_loadCell1->setScale(CELL_SCALE);

  // // Tare pressure transducer processing
  // m_pressTran1->setScale(PT_1600_SCALE);
  // m_pressTran1->tareVolts(EXCITATION_BIAS / 10.0f);  // 0 PSI at 0.1x excitation voltage

  vTaskDelay(pdMS_TO_TICKS(100));
  setupADC_Config();

  uint64_t lastMicros = 0;

  while (true) {
    if (!m_pauseADC) {
      lastMicros = micros();
      queueSample();

      // print adc2 readings to test
      // ESP_LOGD(TAG, "ADC2 MUX 0: %f V", m_adcADS_34->readNewVolt(ADS1X15_REG_CONFIG_MUX_DIFF_0_1));
    }

    vTaskDelay(pdMS_TO_TICKS(1));  // can't starve other tasks
    while ((micros() - lastMicros) < interval_us) {
      // ESP_LOGD(TAG, "Waiting in analogtask");
      vTaskDelay(pdMS_TO_TICKS(1));
    }
  }
}

void Control::queueSample() {
  SampleWithTimestamp sample;
  static uint64_t startMicros = micros();
  sample.timestamp = micros() - startMicros;

  adcADS *adcs[2] = {m_adcADS_12, m_adcADS_34};
  std::array<std::array<ChannelConfig, 4> *, 2> configs = {&m_config->adc1_channels, &m_config->adc2_channels};

  float *sampleValues[8] = {&sample.value1, &sample.value2, &sample.value3, &sample.value4, &sample.value5, &sample.value6, &sample.value7, &sample.value8};

  for (int adcIdx = 0; adcIdx < 2; ++adcIdx) {
    for (int ch = 0; ch < 4; ++ch) {
      int idx = adcIdx * 4 + ch;
      ChannelConfig &cfg = (*configs[adcIdx])[ch];
      if (cfg.mux != -1) {
        float raw = adcs[adcIdx]->readNewVolt(cfg.mux);
        *sampleValues[idx] = m_adcProcessors[idx]->processVtoUnits(raw);
      } else {
        *sampleValues[idx] = 0.0f;
      }
    }
  }

  setLatestSample(sample);

  if (xQueueSend(m_adcQueue, &sample, 0) != pdPASS) {
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

  // Convert std::vector<std::string> to std::vector<String> for SD_Talker
  std::vector<std::string> stdNames = m_config->getChannelNames();
  std::vector<std::string> stdUnits = m_config->getChannelUnits();
  std::vector<String> arduinoNames, arduinoUnits;
  arduinoNames.reserve(stdNames.size());
  arduinoUnits.reserve(stdUnits.size());
  for (const auto &n : stdNames) arduinoNames.push_back(String(n.c_str()));
  for (const auto &u : stdUnits) arduinoUnits.push_back(String(u.c_str()));

  while (true) {
    while (!m_sdTalker->checkFileOpen()) {
      m_sdTalker->startNewLog("/Logs/log", arduinoNames, arduinoUnits);
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

        // while (m_LoRaCom->isQueued(commandSeqID)) {
        //   vTaskDelay(pdMS_TO_TICKS(10));
        // }

        if (requireAck) {
          while (!m_LoRaCom->isAcked(commandSeqID)) vTaskDelay(pdMS_TO_TICKS(10));
        }

        // ESP_LOGI(TAG, "LINE 355");

        m_pauseADC = true;
        if (payload.paramType == 0) {
          // Float parameter
          m_commander->runCommand(payload.commandID, payload.paramFloat);
        } else {
          // String parameter
          m_commander->runCommand(payload.commandID, payload.paramString);
        }
        m_pauseADC = false;
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
        m_pauseADC = true;
        if (payload.paramType == 0) {
          // Float parameter
          m_commander->runCommand(payload.commandID, payload.paramFloat);
        } else {
          // String parameter
          m_commander->runCommand(payload.commandID, payload.paramString);
        }
        m_pauseADC = false;
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

  // Cache channel names for efficiency (static, only initialized once)
  static bool namesInitialized = false;
  static char channelNames[8][24];  // 24 chars per name (adjust as needed)
  if (!namesInitialized) {
    for (int i = 0; i < 4; ++i) {
      const auto &ch = m_config->adc1_channels[i];
      if (!ch.name.empty()) {
        strncpy(channelNames[i], ch.name.c_str(), sizeof(channelNames[i]) - 1);
        channelNames[i][sizeof(channelNames[i]) - 1] = '\0';
      } else {
        snprintf(channelNames[i], sizeof(channelNames[i]), "IN%d", i + 1);
      }
    }
    for (int i = 0; i < 4; ++i) {
      const auto &ch = m_config->adc2_channels[i];
      if (!ch.name.empty()) {
        strncpy(channelNames[i + 4], ch.name.c_str(), sizeof(channelNames[i + 4]) - 1);
        channelNames[i + 4][sizeof(channelNames[i + 4]) - 1] = '\0';
      } else {
        snprintf(channelNames[i + 4], sizeof(channelNames[i + 4]), "IN%d", i + 5);
      }
    }
    namesInitialized = true;
  }

  while (true) {
    StatusPayload payload;
    payload.rssi = static_cast<int8_t>(m_LoRaCom->getRssi());
    payload.batteryVoltage = m_battMonitor->getScaledVoltage(/*num_readings*/ 20);
    payload.status = deviceStatus::STATUS_OK;

    SampleWithTimestamp sample;
    getLatestSample(sample);
    payload.IN1 = sample.value1;
    payload.IN2 = sample.value2;
    payload.IN3 = sample.value3;
    payload.IN4 = sample.value4;
    payload.IN5 = sample.value5;
    payload.IN6 = sample.value6;
    payload.IN7 = sample.value7;
    payload.IN8 = sample.value8;

    memcpy(msg.payload, &payload, sizeof(payload));

    // Use a preallocated buffer for the status message
    char statusMsg[256];
    int len = snprintf(statusMsg, sizeof(statusMsg), "status ID:%d RSSI:%d battVoltage:%.3f status:%d", msg.senderID, payload.rssi, payload.batteryVoltage, payload.status);
    float values[8] = {payload.IN1, payload.IN2, payload.IN3, payload.IN4, payload.IN5, payload.IN6, payload.IN7, payload.IN8};
    for (int i = 0; i < 8; ++i) {
      // Append each channel name and value
      len += snprintf(statusMsg + len, sizeof(statusMsg) - len, " %s:%.2f", channelNames[i], values[i]);
      if (len >= (int)sizeof(statusMsg) - 1) break;
    }
    // Ensure newline and null-termination
    if (len < (int)sizeof(statusMsg) - 2) {
      statusMsg[len++] = '\n';
      statusMsg[len] = '\0';
    } else {
      statusMsg[sizeof(statusMsg) - 2] = '\n';
      statusMsg[sizeof(statusMsg) - 1] = '\0';
    }

    m_serialCom->sendData(statusMsg);

    if (m_LoRaCom->enqueueMessage(msg, false)) ESP_LOGD(TAG, "Adding to transmit queue...");

    // checkTaskStack();
    vTaskDelay(pdMS_TO_TICKS(status_Interval));
  }
}

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

void Control::setLatestSample(const SampleWithTimestamp &sample) {
  SemaphoreGuard Guard_adc(m_latestSampleMutex);
  if (Guard_adc.acquired()) {
    m_latestSample = sample;  // Update the latest sample with the new data
  } else {
    ESP_LOGW(TAG, "Failed to acquire mutex for latest sample update");
  }
}

void Control::getLatestSample(SampleWithTimestamp &sample) {
  SemaphoreGuard Guard_adc(m_latestSampleMutex);
  if (Guard_adc.acquired()) {
    sample = m_latestSample;  // Copy the latest sample data
  } else {
    ESP_LOGW(TAG, "Failed to acquire mutex for latest sample retrieval");
  }
}

void Control::setupADC_Channels(adcADS *adc, std::array<ChannelConfig, 4> &channels, int processorOffset) {
  for (int i = 0; i < 4; ++i) {
    ChannelConfig &ch = channels[i];
    m_adcProcessors[i + processorOffset] = new adcProcessor();
    m_adcProcessors[i + processorOffset]->setScale(ch.scale_factor);

    ch.mux = -1;
    if (ch.mode == "differential") {
      if (ch.inputs == std::vector<int>{0, 1})
        ch.mux = ADS1X15_REG_CONFIG_MUX_DIFF_0_1;
      else if (ch.inputs == std::vector<int>{2, 3})
        ch.mux = ADS1X15_REG_CONFIG_MUX_DIFF_2_3;
    } else if (ch.mode == "single_ended" && ch.inputs.size() == 1) {
      if (ch.inputs[0] == 0)
        ch.mux = ADS1X15_REG_CONFIG_MUX_SINGLE_0;
      else if (ch.inputs[0] == 1)
        ch.mux = ADS1X15_REG_CONFIG_MUX_SINGLE_1;
      else if (ch.inputs[0] == 2)
        ch.mux = ADS1X15_REG_CONFIG_MUX_SINGLE_2;
      else if (ch.inputs[0] == 3)
        ch.mux = ADS1X15_REG_CONFIG_MUX_SINGLE_3;
    }

    float tareValue = 0.0f;
    if (ch.tare_bias.auto_tare && ch.mux != -1) {
      tareValue = adc->getAverageVolt(200, ch.mux);
    } else {
      tareValue = ch.tare_bias.value;
    }
    m_adcProcessors[i + processorOffset]->tareVolts(tareValue);
  }
}

void Control::setupADC_Config() {
  setupADC_Channels(m_adcADS_12, m_config->adc1_channels, 0);
  setupADC_Channels(m_adcADS_34, m_config->adc2_channels, 4);
  ESP_LOGI(TAG, "ADC configuration setup complete");
}