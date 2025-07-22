#pragma once

#include <Arduino.h>

#include "esp_log.h"

class adcBase {
 public:
  // Read latest value from the ADC
  // virtual float readNewVolt() = 0;

  // Check if the ADC is ready for reading
  virtual bool isReady() const = 0;

  // Get the resolution of the ADC
  virtual int getResolution() const = 0;

  // Set up ADC for continuous reading, we might neeed DMAs and ISRs
  virtual void startContinuous() = 0;

  // Set the gain for the ADC
  virtual bool setGain(int gain) = 0;

  virtual bool setDataRate(uint16_t rate) = 0;

 protected:
  float m_lastResultV = 0;
  const char *TAG = "adcBase";  // Log tag for ADC base class
  int m_rate;

 private:
};
