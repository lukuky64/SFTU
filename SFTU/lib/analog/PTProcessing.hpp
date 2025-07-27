#pragma once
#include <Arduino.h>

class PTProcessing {
 public:
  PTProcessing();
  float processVtoPSI(float voltage, float units_per_V = -1.0f);
  void tareVolts(float voltage);
  // void calibrate(float current_presure, float voltage);
  void setScale(float scale);

 private:
  float m_units_per_V;
  float m_Voffset;  // Offset voltage for calibration
};
