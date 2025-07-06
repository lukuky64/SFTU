#include "control.hpp"

Control::Control() {
  m_ANALOG_I2C_BUS = new TwoWire(0);
  m_I2C_BUS = new TwoWire(1);

  m_serialCom = new SerialCom();  // Initialize SerialCom instance
  m_LoRaCom = new LoRaCom();      // Initialize LoRaCom instance

  m_adcADS = new adcADS(*m_ANALOG_I2C_BUS);  // Initialize adcADS instance

#ifdef SFTU
  m_actuation = new Actuation(PCA6408A_SLAVE_ADDRESS_L,
                              PCA6408A_SLAVE_ADDRESS_H, *m_I2C_BUS);
  m_commander = new Commander(m_serialCom, m_LoRaCom, m_actuation);
#else
  m_commander = new Commander(m_serialCom, m_LoRaCom);
  m_saveFlash = new SaveFlash(m_serialCom);  // Initialize SaveFlash instance
#endif
}

void Control::setup() {
#ifdef SFTU
  m_I2C_BUS->begin(I2C2_SDA, I2C2_SCL, 100000);
  m_ANALOG_I2C_BUS->begin(I2C1_SDA, I2C1_SCL, 100000);  // Initialize I2C buses
  m_actuation->init();

  m_adcADS->init(ADS0_ADDR);  // Use ADS0 address
#else
#endif

  m_serialCom->init(115200);  // Initialize serial communication

  bool loraSuccess =
      m_LoRaCom->begin<SX1276>(SPI_CLK_RF, SPI_MISO_RF, SPI_MOSI_RF, SPI_CS_RF,
                               RF_DIO, RF_RST, 915.0f, 20);

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
  if (SerialTaskHandle != nullptr) {
    vTaskDelete(SerialTaskHandle);
  }

  if (LoRaTaskHandle != nullptr) {
    vTaskDelete(LoRaTaskHandle);
  }

  if (StatusTaskHandle != nullptr) {
    vTaskDelete(StatusTaskHandle);
  }

  if (heartBeatTaskHandle != nullptr) {
    vTaskDelete(heartBeatTaskHandle);
  }

  if (analogTaskHandle != nullptr) {
    vTaskDelete(analogTaskHandle);
  }

  // Create new tasks for serial data handling, LoRa data handling, and status
  // Higher priority = higher number, priorities should be 1-3 for user tasks
  xTaskCreate(
      [](void *param) { static_cast<Control *>(param)->serialDataTask(); },
      "SerialDataTask", 4096, this, 2, &SerialTaskHandle);

  xTaskCreate(
      [](void *param) { static_cast<Control *>(param)->loRaDataTask(); },
      "LoRaDataTask", 4096, this, 2, &LoRaTaskHandle);

  xTaskCreate([](void *param) { static_cast<Control *>(param)->statusTask(); },
              "StatusTask", 4096, this, 1, &StatusTaskHandle);

  xTaskCreate(
      [](void *param) { static_cast<Control *>(param)->heartBeatTask(); },
      "HeartBeatTask", 2048, this, 1, &heartBeatTaskHandle);

  xTaskCreate([](void *param) { static_cast<Control *>(param)->analogTask(); },
              "AnalogTask", 4096, this, 3, &analogTaskHandle);

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

void Control::analogTask() {
  while (true) {
    float data = m_adcADS->readVolt();  // Read the voltage from the ADC
    m_serialCom->sendData("ADC Data: ");
    m_serialCom->sendData(
        String(data, 4).c_str());  // Send the data with 4 decimal places
    m_serialCom->sendData("\n");

    vTaskDelay(pdMS_TO_TICKS(100));  // Delay for 1 second
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

    // Use a small delay instead of yield() to be more cooperative
    vTaskDelay(pdMS_TO_TICKS(10));  // 10ms delay allows other tasks to run
  }
}

void Control::loRaDataTask() {
  char buffer[128];  // Buffer to store incoming data\

  int rxIndex = 0;   // Index to track the length of the received message

  while (true) {
    // Check for incoming data from the LoRa interface
    if (m_LoRaCom->getMessage(buffer, sizeof(buffer))) {
      // if (m_LoRaCom->getMessage(buffer, sizeof(buffer), &rxIndex)){
      ESP_LOGD(TAG, "Received: %s", buffer);  // Log the received data
      interpretMessage(buffer, false);        // Process the message
      // Send the received data over serial
      m_serialCom->sendData("Received: <");
      m_serialCom->sendData(buffer);
      m_serialCom->sendData(">\n");

      memset(buffer, 0, sizeof(buffer));
    }

    // Use a small delay instead of yield() to be more cooperative
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void Control::statusTask() {
  static unsigned long lastStatusTime = 0;

  while (true) {
    // Process any pending LoRa operations first
    // m_LoRaCom->processOperations();

    int32_t rssi = m_LoRaCom->getRssi();
    String msg = String("status ") + "ID:" + deviceID +
                 " RSSI:" + String(rssi) +
                 " batteryLevel:" + String(m_batteryLevel) + " mode:" + m_mode +
                 " status:" + m_status;

    // Send over serial first (this should be fast)
    m_serialCom->sendData(((msg + "\n").c_str()));

    // Try LoRa transmission with timeout protection
    ESP_LOGD(TAG, "Starting LoRa transmission...");
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