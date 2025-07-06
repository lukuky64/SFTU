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
<<<<<<< HEAD
  float readNewVolt() override;

  float getLastVolt();
=======
  float readVolt() override;
>>>>>>> ba8e11dd8bb98570192ebf809580b7a0ed7731e4

  // Check if the ADC is ready for reading
  bool isReady() const override;

  // Get the resolution of the ADC
  int getResolution() const override;

  // Set up ADC for continuous reading, we might need DMAs and ISRs
<<<<<<< HEAD
  void startContinuous(xQueueHandle &adcQueue) override;
=======
  void startContinuous() override;
>>>>>>> ba8e11dd8bb98570192ebf809580b7a0ed7731e4

  // Set the gain for the ADC
  bool setGain(int gain) override;

  // Set the data rate for the ADC
  bool setDataRate(uint16_t rate) override;

  // set up the configuration for the ADC inputs
  void setInputConfig(adsGain_t gain = GAIN_TWOTHIRDS,
                      uint8_t dataRate = RATE_ADS1115_128SPS,
                      int mux = ADS1X15_REG_CONFIG_MUX_DIFF_0_1);

 private:
  Adafruit_ADS1115 *m_adc;
  int m_mux;
<<<<<<< HEAD
=======
  int m_rate;
>>>>>>> ba8e11dd8bb98570192ebf809580b7a0ed7731e4
  TwoWire *m_I2C_BUS;

  // Add any private members or methods needed for the ADS ADC implementation
};