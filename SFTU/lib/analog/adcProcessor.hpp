#pragma once
#include <Arduino.h>

class adcProcessor {
 public:
  adcProcessor();
  float processVtoUnits(float voltage, float units_per_V = -1.0f);
  void tareVolts(float voltage);
  void calibrate(float realUnits, float voltage);
  void setScale(float scale);

 private:
  float m_units_per_V;
  float m_Voffset;
  static constexpr const char *TAG = "adcProcessor";
};
