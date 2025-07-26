#pragma once
#include <Arduino.h>

class ADCprocessing {
 public:
  ADCprocessing();
  float processVtoN(float voltage, float units_per_V = -1.0f);
  void tareVolts(float voltage);
  void calibrate(float objectMass, float voltage);
  void setScale(float scale);

 private:
 float m_units_per_V;
  float m_Voffset;                // Offset voltage for calibration
  const float m_gravity = 9.81f;  // Acceleration due to gravity in m/s^2
};
