#pragma once
#include <Arduino.h>

class ADCprocessing {
 public:
  ADCprocessing(float unitsPerVolt);
  float processVtoN(float voltage);
  void tareVolts(float voltage);
  void calibrate(float objectMass, float voltage);
    void setScale(float scale);

 private:
  float m_Units_per_V;
  float m_Voffset;                // Offset voltage for calibration
  const float m_gravity = 9.81f;  // Acceleration due to gravity in m/s^2
};
