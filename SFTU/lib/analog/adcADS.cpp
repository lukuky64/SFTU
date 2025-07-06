#include "adcADS.hpp"

adcADS::adcADS(TwoWire &Wire) {
  m_I2C_BUS = &Wire;
  m_adc = new Adafruit_ADS1115();
  //
}

void adcADS::init(uint8_t addr) {
  if (!m_adc->begin(addr, m_I2C_BUS)) {
    Serial.println("Failed to initialise ADS1115.");
  }

  setInputConfig();

  Serial.println("ADS1115 initialised successfully!");
}

void adcADS::setInputConfig(adsGain_t gain, uint8_t dataRate, int mux) {
  m_adc->setGain(gain);
  m_adc->setDataRate(dataRate);
  m_mux = mux;
}

void adcADS::startContinuous() {
  m_adc->startADCReading(m_mux, true);
  //
}

float adcADS::readVolt() {
  m_adc->startADCReading(m_mux, false);

  // Wait for the conversion to complete
  while (!m_adc->conversionComplete());

  uint16_t rawADC = m_adc->getLastConversionResults();
  m_lastResultV = m_adc->computeVolts(rawADC);
  return m_lastResultV;
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
    ESP_LOGE(
        TAG,
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
      adsGain =
          GAIN_TWOTHIRDS;  // +/- 6.144V range (limited to VCC + 0.3V max!)
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
      ESP_LOGE(TAG, "Invalid gain: %d. Valid gains are 1, 2, 4, 8, or 16",
               gain);
      return false;
  }

  m_adc->setGain(adsGain);
  ESP_LOGI(TAG, "Gain set to %d", gain);
  return true;
}