#include "ADCprocessing.hpp"

ADCprocessing::ADCprocessing(float unitsPerVolt) {
  m_Units_per_V = unitsPerVolt;
  m_Voffset = 0.0f;  // Offset voltage for calibration
}

float ADCprocessing::processVtoN(float voltage) {
  return (voltage - m_Voffset) * m_Units_per_V;
  //
}

void ADCprocessing::tareVolts(float voltage) {
  m_Voffset = voltage;
  //
}

// use commander to call this function
void ADCprocessing::calibrate(float objectMass, float voltage) {
  ESP_LOGI("ADCprocessing", "rawVoltage: %.3f V, Voffset: %.3f V", voltage, m_Voffset);

  float taredVoltage = voltage - m_Voffset;
  ESP_LOGI("ADCprocessing", "Calibrating with object mass: %.3f kg, tared voltage: %.3f V", objectMass, taredVoltage);
  float objectForce = objectMass * m_gravity;
  m_Units_per_V = objectForce / (voltage - m_Voffset);
  ESP_LOGI("ADCprocessing", "Calibration complete: Units_per_V = %.3f N/V", m_Units_per_V);
}

void ADCprocessing::setScale(float scale) {
  m_Units_per_V = scale;
  ESP_LOGI("ADCprocessing", "Cell scale set to: %.3f N/V", m_Units_per_V);
}