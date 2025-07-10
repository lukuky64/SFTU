
#include "control.hpp"

Control::Control() {
  m_ANALOG_I2C_BUS = new TwoWire(0);
  m_I2C_BUS = new TwoWire(1);
  m_SPI_BUS = new SPIClass();

  m_serialCom = new SerialCom();
  m_LoRaCom = new LoRaCom();

  m_adcADS = new adcADS(*m_ANALOG_I2C_BUS);

  m_sdTalker = new SD_Talker();

  m_battMonitor = new BattMonitor(VOLTAGE_SENSE, 4.032f);

#ifdef SFTU
  m_actuation = new Actuation(PCA6408A_SLAVE_ADDRESS_L,
                              PCA6408A_SLAVE_ADDRESS_H, *m_I2C_BUS);
  m_commander = new Commander(m_serialCom, m_LoRaCom, m_actuation, m_adcADS);

  m_display = new Display();
#else
  m_commander = new Commander(m_serialCom, m_LoRaCom);
  m_saveFlash = new SaveFlash(m_serialCom);
#endif
}

void Control::setup() {
#ifdef SFTU
  m_I2C_BUS->begin(I2C2_SDA, I2C2_SCL);
  m_ANALOG_I2C_BUS->begin(I2C1_SDA, I2C1_SCL);
  m_SPI_BUS->begin(SPI_CLK_SD, SPI_MISO_SD, SPI_MOSI_SD);

  m_I2C_BUS->setClock(400'000);
  m_ANALOG_I2C_BUS->setClock(400'000);
  m_SPI_BUS->setFrequency(40'000'000);

  m_actuation->init();
  m_sdTalker->begin(SD_CD, SPI_CS_SD, *m_SPI_BUS);  // Initialize SD card
  m_adcADS->init(ADS0_ADDR);                        // Use ADS0 address
  m_battMonitor->init();  // Initialize battery monitor

  m_display->init(*m_I2C_BUS);  // Initialize the display

  // from testing, queue goes up to ~200 samples during sd write
  m_adcQueue = xQueueCreate(512, sizeof(SampleWithTimestamp));

#else
#endif

  m_serialCom->init(115200);  // Initialize serial communication

  bool loraSuccess =
      m_LoRaCom->begin<SX1276>(SPI_CLK_RF, SPI_MISO_RF, SPI_MOSI_RF, SPI_CS_RF,
                               RF_DIO, RF_RST, 915.0f, 20);
  m_LoRaCom->setRadioType(RADIO_SX127X);

  if (!loraSuccess) {
    ESP_LOGE(TAG,
             "LoRa initialization FAILED! Check your hardware connections.");
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
  xTaskCreate(
      [](void *param) { static_cast<Control *>(param)->serialDataTask(); },
      "SerialDataTask", 4096, this, 2, &m_taskHandles.SerialTaskHandle);

  xTaskCreate(
      [](void *param) { static_cast<Control *>(param)->loRaDataTask(); },
      "LoRaDataTask", 4096, this, 2, &m_taskHandles.LoRaTaskHandle);

  xTaskCreate([](void *param) { static_cast<Control *>(param)->statusTask(); },
              "StatusTask", 4096, this, 1, &m_taskHandles.StatusTaskHandle);

  xTaskCreate(
      [](void *param) { static_cast<Control *>(param)->heartBeatTask(); },
      "HeartBeatTask", 4096, this, 1, &m_taskHandles.heartBeatTaskHandle);

  xTaskCreate([](void *param) { static_cast<Control *>(param)->analogTask(); },
              "AnalogTask", 8192, this, 3, &m_taskHandles.analogTaskHandle);

  xTaskCreate([](void *param) { static_cast<Control *>(param)->sdTask(); },
              "sdTask", 8192, this, 3, &m_taskHandles.sdTaskHandle);

  xTaskCreate([](void *param) { static_cast<Control *>(param)->displayTask(); },
              "sdTask", 4096, this, 1, &m_taskHandles.displayTaskHandle);

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
  vTaskDelay(pdMS_TO_TICKS(500));
  // m_display->dim(true);
  m_display->begin();  // Begin the display

  while (true) {
    // Update the display with the current force value
    if (m_adcQueue != nullptr) {
      SampleWithTimestamp sample;
      if (xQueuePeek(m_adcQueue, &sample, pdMS_TO_TICKS(10))) {
        m_display->drawForce(sample.value, true);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(40));
  }
}

void Control::analogTask() {
  m_adcADS->setInputConfig(GAIN_FOUR, RATE_ADS1115_860SPS,
                           ADS1X15_REG_CONFIG_MUX_DIFF_0_1);
  m_adcADS->setDataRate(adcSPS);
  m_adcADS->startContinuous();

  uint64_t interval_us = (uint64_t)(1e6 / (double)adcSPS);
  TickType_t interval_ticks = pdMS_TO_TICKS((uint32_t)(interval_us / 1000));

  m_adcADS->getAverageVolt(100);  // throw away some data first
  float averageSample = m_adcADS->getAverageVolt(500);
  tareVolts(averageSample);

  uint64_t lastMicros = 0;
  while (true) {
    lastMicros = micros();
    queueSample();
    vTaskDelay(interval_ticks);  // Yield to other tasks

    while ((micros() - lastMicros) < interval_us);
  }
}

void Control::queueSample() {
  SampleWithTimestamp sample;
  static uint64_t startMicros = micros();
  sample.timestamp = micros() - startMicros;
  sample.value = processVtoN(m_adcADS->getLastVolt());
  xQueueSend(m_adcQueue, &sample, 0);
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
    if (count >= blockSize ||
        (count > 0 && (now - lastBlockTime) >= blockTimeout)) {
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

  while (true) {
    // Check for incoming data from the serial interface
    if (m_serialCom->getData(buffer, sizeof(buffer), &rxIndex)) {
      ESP_LOGI(TAG, "Received: %s", buffer);  // Log the received data
      interpretMessage(buffer, true);         // Process the message
      // clear the buffer for the next message
      memset(buffer, 0, sizeof(buffer));
      rxIndex = 0;  // Reset the index
    }

    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void Control::loRaDataTask() {
  char buffer[128];  // Buffer to store incoming data\

  int rxIndex = 0;   // Index to track the length of the received message

  pinMode(INDICATOR_LED2, OUTPUT);  // Set LED pin as output

  while (true) {
    // Check for incoming data from the LoRa interface
    if (m_LoRaCom->getMessage(buffer, sizeof(buffer))) {
      digitalWrite(INDICATOR_LED2, !digitalRead(INDICATOR_LED2));  // Toggle LED
      // if (m_LoRaCom->getMessage(buffer, sizeof(buffer), &rxIndex)){
      ESP_LOGD(TAG, "Received: %s", buffer);  // Log the received data
      interpretMessage(buffer, false);        // Process the message
      // Send the received data over serial
      m_serialCom->sendData("Received: <");
      m_serialCom->sendData(buffer);
      m_serialCom->sendData(">\n");

      memset(buffer, 0, sizeof(buffer));
    }

    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void Control::statusTask() {
  static unsigned long lastStatusTime = 0;

  while (true) {
    m_batteryVoltage = m_battMonitor->getScaledVoltage(/*num_readings*/ 20);

    int32_t rssi = m_LoRaCom->getRssi();
    String msg = String("status ") + "ID:" + deviceID +
                 " RSSI:" + String(rssi) +
                 " battVoltage:" + String(m_batteryVoltage) +
                 " mode:" + m_mode + " status:" + m_status;

    // Send over serial first (this should be fast)
    m_serialCom->sendData(((msg + "\n").c_str()));

    // Try LoRa transmission with timeout protection
    ESP_LOGD(TAG, "Starting LoRa transmission...");

    while (m_LoRaCom->checkRx()) vTaskDelay(pdMS_TO_TICKS(1));
    m_LoRaCom->sendMessage(msg.c_str());
    vTaskDelay(pdMS_TO_TICKS(status_Interval));
  }
}

void Control::interpretMessage(const char *buffer, bool relayMsgLoRa) {
  m_commander->setCommand(buffer);  // Set the command in the commander
  char *token = m_commander->readAndRemove();

  // eg: "command update gain 22"
  // eg: "status <deviceID> <RSSI> <batteryLevel> <mode> <status>"
  // eg: "data <payload>"

  if (c_cmp(token, "command")) {
    if (relayMsgLoRa) {
      // send to other devices to sync parameters
      while (m_LoRaCom->checkRx()) vTaskDelay(pdMS_TO_TICKS(1));
      m_LoRaCom->sendMessage(buffer);
    }
    // should probably wait for a success reply before changing THIS device
    ESP_LOGD(TAG, "Processing command: %s", buffer);
    m_commander->checkCommand();
  } else if (c_cmp(token, "data")) {
    processData(buffer);
  } else if (c_cmp(token, "status")) {
    processData(buffer);
  } else if (c_cmp(token, "help")) {
    ESP_LOGI(TAG,
             "Message format: <type> <data1> <data2> ...\n"
             "Valid types:\n"
             "  - command: for device control\n"
             "  - data: for data transmission\n"
             "  - message: for standard messages\n"
             "  - flash: to print and auto erase logs\n"
             "  - status: for device status\n"
             "  - help: for displaying help information");
  }

#ifdef SFTU
#else
  else if (c_cmp(token, "flash")) {
    m_saveFlash->readFile();
    m_saveFlash->removeFile();  // Update the flash storage
    m_saveFlash->begin();       // Reinitialize the flash storage
  }
#endif
}

void Control::processData(const char *buffer) {
  // Process the data message
  ESP_LOGD(TAG, "Processing data");

  // remove the "data" prefix
  const char *dataStart = strchr(buffer, ' ') + 1;  // Find the first space
  if (dataStart == nullptr) {
    ESP_LOGE(TAG, "Invalid data format: %s", buffer);
    return;  // Invalid format, return early
  }

  m_serialCom->sendData(buffer);  // Send the data part over serial
  m_serialCom->sendData("\n");

#ifdef SFTU
#else
  // save dataStart to flash
  m_saveFlash->writeData((dataStart + String("\n")).c_str());
#endif

  ESP_LOGI(TAG, "Data processing complete");
}

void Control::checkTaskStack() {
  // print how much stack is left on each task
  for (const auto &task : m_taskHandleMap) {
    if (task.handle != nullptr && *task.handle != nullptr) {
      UBaseType_t stackHighWaterMark =
          uxTaskGetStackHighWaterMark(*task.handle);
      ESP_LOGI(TAG, "%s: Stack high water mark: %u", task.name.c_str(),
               stackHighWaterMark);
    } else {
      ESP_LOGI(TAG, "%s: Task handle is null", task.name.c_str());
    }
  }
}