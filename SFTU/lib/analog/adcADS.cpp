#include "adcADS.hpp"

adcADS::adcADS(TwoWire &Wire) {
  m_I2C_BUS = &Wire;
  m_adc = new Adafruit_ADS1115();
  m_adcMutex = xSemaphoreCreateMutex();
  //
}

void adcADS::init(uint8_t addr) {
  // Activate I2C high-speed mode by sending the Hs controller code (0x08)
  m_I2C_BUS->beginTransmission(0x08);  // Hs controller code, not acknowledged
  m_I2C_BUS->endTransmission();

  if (!m_adc->begin(addr, m_I2C_BUS)) {
    Serial.println("Failed to initialise ADS1115.");
  }

  setInputConfig();

  Serial.println("ADS1115 initialised successfully!");
}

void adcADS::setInputConfig(adsGain_t gain, uint8_t dataRate) {
  m_adc->setGain(gain);
  m_adc->setDataRate(dataRate);
}

void adcADS::startContinuous(const uint16_t mux) {
  // Start continuous ADC reading
  continuousMode = true;
  m_adc->startADCReading(mux, continuousMode);
}

float adcADS::readNewVolt(const uint16_t mux) {
  SemaphoreGuard Guard_adc(m_adcMutex);
  if (Guard_adc.acquired()) {
    continuousMode = false;
    m_adc->startADCReading(mux, continuousMode);

    // Wait for the conversion to complete
    while (!m_adc->conversionComplete()) {
      // NOTE: This slows things slightly, but atleast we aren't blocking
      vTaskDelay(pdMS_TO_TICKS(1));  // Yield to other tasks
    }
    // ESP_LOGD(TAG, "ADC conversion complete for mux %d", mux);

    m_lastResultV = m_adc->computeVolts(m_adc->getLastConversionResults());
    return m_lastResultV;
  } else {
    ESP_LOGE(TAG, "Failed to acquire ADC mutex in getAverageVolt");
    return 0.0f;  // Return 0 if mutex acquisition failed
  }
}

float adcADS::getLastVolt() {
  SemaphoreGuard Guard_adc(m_adcMutex);
  if (Guard_adc.acquired()) {
    m_lastResultV = m_adc->computeVolts(m_adc->getLastConversionResults());
    return m_lastResultV;
  } else {
    ESP_LOGE(TAG, "Failed to acquire ADC mutex in getAverageVolt");
    return 0.0f;  // Return 0 if mutex acquisition failed
  }
}

bool adcADS::setDataRate(uint16_t rate) {
  if (rate <= 8) {
    m_rate = RATE_ADS1115_8SPS;
  } else if (rate <= 16) {
    m_rate = RATE_ADS1115_16SPS;
  } else if (rate <= 32) {
    m_rate = RATE_ADS1115_32SPS;
  } else if (rate <= 64) {
    m_rate = RATE_ADS1115_64SPS;
  } else if (rate <= 128) {
    m_rate = RATE_ADS1115_128SPS;
  } else if (rate <= 250) {
    m_rate = RATE_ADS1115_250SPS;
  } else if (rate <= 475) {
    m_rate = RATE_ADS1115_475SPS;
  } else if (rate <= 860) {
    m_rate = RATE_ADS1115_860SPS;
  } else {
    ESP_LOGE(TAG,
             "Invalid data rate: %d. Valid rates are 8, 16, 32, 64, 128, 250, 475, "
             "or 860 SPS.",
             rate);
    return false;
  }

  m_adc->setDataRate(m_rate);
  ESP_LOGI(TAG, "Data rate set to %d SPS", m_rate);
  return true;
}

bool adcADS::isReady() const { return m_adc->conversionComplete(); }

int adcADS::getResolution() const {
  // ADS1115 has 16-bit resolution
  return 16;
}

bool adcADS::setGain(int gain) {
  adsGain_t adsGain;

  switch (gain) {
    case 1:
      adsGain = GAIN_TWOTHIRDS;  // +/- 6.144V range (limited to VCC + 0.3V max!)
      break;
    case 2:
      adsGain = GAIN_ONE;  // +/- 4.096V range (limited to VCC + 0.3V max!)
      break;
    case 4:
      adsGain = GAIN_TWO;  // +/- 2.048V range (limited to VCC + 0.3V max!)
      break;
    case 8:
      adsGain = GAIN_FOUR;  // +/- 1.024V range (limited to VCC + 0.3V max!)
      break;
    case 16:
      adsGain = GAIN_EIGHT;  // +/- 0.512V range (limited to VCC + 0.3V max!)
      break;
    default:
      ESP_LOGE(TAG, "Invalid gain: %d. Valid gains are 1, 2, 4, 8, or 16", gain);
      return false;
  }

  m_adc->setGain(adsGain);
  ESP_LOGI(TAG, "Gain set to %d", gain);
  return true;
}

float adcADS::getAverageVolt(uint16_t numSamples, const uint16_t mux) {
  float averageSample = 0.0f;
  ESP_LOGD(TAG, "Acquired ADC mutex in getAverageVolt");
  for (int i = 0; i < numSamples; ++i) {
    if (continuousMode) {
      averageSample += getLastVolt();
    } else {
      averageSample += readNewVolt(mux);
      // ESP_LOGD(TAG, "Sample %d: %.3f V", i + 1, m_lastResultV);
    }
    vTaskDelay(pdMS_TO_TICKS(1));
  }
  averageSample /= numSamples;
  ESP_LOGD(TAG, "Average sample voltage: %.3f V", averageSample);
  return averageSample;
}