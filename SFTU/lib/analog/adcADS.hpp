#pragma once

#include <Adafruit_ADS1X15.h>

#include "adcBase.hpp"

#define ADS0_ADDR 0x48
#define ADS1_ADDR 0x49

class adcADS : public adcBase {
 public:
  adcADS(TwoWire &Wire);

  // Initialize the ADC
  void init(uint8_t addr);

  // Read latest value from the ADC
  float readNewVolt() override;

  float getLastVolt();

  // Check if the ADC is ready for reading
  bool isReady() const override;

  // Get the resolution of the ADC
  int getResolution() const override;

  // Set up ADC for continuous reading, we might need DMAs and ISRs
  void startContinuous() override;

  // Set the gain for the ADC
  bool setGain(int gain) override;

  // Set the data rate for the ADC
  bool setDataRate(uint16_t rate) override;

  // set up the configuration for the ADC inputs
  void setInputConfig(adsGain_t gain = GAIN_TWOTHIRDS,
                      uint8_t dataRate = RATE_ADS1115_128SPS,
                      int mux = ADS1X15_REG_CONFIG_MUX_DIFF_0_1);

  float getAverageVolt(uint16_t numSamples);

 private:
  Adafruit_ADS1115 *m_adc;
  int m_mux;
  TwoWire *m_I2C_BUS;
  bool continuousMode = false;

  // Add any private members or methods needed for the ADS ADC implementation
};