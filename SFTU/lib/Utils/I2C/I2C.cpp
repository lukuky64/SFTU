#include "I2C.hpp"

bool i2cBegin(uint8_t sda, uint8_t scl, uint32_t frequency, TwoWire &wire_) {
  // Start I2C with given SDA and SCL pins
  if (!wire_.begin(sda, scl)) {
    Serial.printf("I2C wire_.begin() Failed: SDA: %d, SCL: %d\n", sda, scl);
    return false;
  }

  // Attempt to set I2C clock frequency
  if (!wire_.setClock(frequency)) {
    Serial.printf("I2C wire_.setClock() Failed: Frequency: %lu Hz\n",
                  frequency);
    return false;
  }

  // If execution reaches here, it means wire_.begin() was successful and
  // wire_.setClock() was either successful or failed silently.
  Serial.printf("I2C Init Successful: SDA: %d, SCL: %d, Frequency: %lu Hz\n",
                sda, scl, frequency);
  return true;
}

uint8_t i2cReadByte(uint8_t address, uint8_t subAddress, TwoWire &wire_) {
  uint8_t data;
  wire_.beginTransmission(address);
  wire_.write(subAddress);
  wire_.endTransmission(false);
  wire_.requestFrom(address, (uint8_t)1);
  data = wire_.read();
  return data;
}

void i2cWriteByte(uint8_t address, uint8_t subAddress, uint8_t data,
                  TwoWire &wire_) {
  wire_.beginTransmission(address);
  wire_.write(subAddress);
  wire_.write(data);
  wire_.endTransmission();
}

void i2cReadBytes(uint8_t address, uint8_t subAddress, uint8_t count,
                  uint8_t *dest, TwoWire &wire_) {
  wire_.beginTransmission(address);
  wire_.write(subAddress);
  wire_.endTransmission(false);
  wire_.requestFrom(address, count);
  for (uint8_t i = 0; wire_.available(); i++) {
    dest[i] = wire_.read();
  }
}

void i2cWriteBytes(uint8_t address, uint8_t subAddress, uint8_t count,
                   const uint8_t *data, TwoWire &wire_) {
  wire_.beginTransmission(address);
  wire_.write(subAddress);
  for (uint8_t i = 0; i < count; i++) {
    wire_.write(data[i]);
  }
  wire_.endTransmission();
}

void i2cScan(TwoWire &wire_) {
  byte error, address;
  int devices = 0;

  ESP_LOGI("I2C", "Scanning I2C bus...");

  for (address = 1; address < 127; address++) {
    wire_.beginTransmission(address);
    error = wire_.endTransmission();

    if (error == 0) {
      ESP_LOGI("I2C", "I2C device found at address 0x%02X", address);
      devices++;
    } else if (error == 4) {
      ESP_LOGE("I2C", "Unknown error at address 0x%02X", address);
    }
  }

  if (devices == 0) {
    ESP_LOGW("I2C", "No I2C devices found");
  } else {
    ESP_LOGI("I2C", "%d I2C device(s) found", devices);
  }

  delay(2000);  // Wait before next scan
}