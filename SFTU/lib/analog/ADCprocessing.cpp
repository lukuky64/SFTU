#include "ADCprocessing.hpp"

ADCprocessing::ADCprocessing() {
  m_units_per_V = 0.0f;  // Initialize units per volt
  m_Voffset = 0.0f;      // Offset voltage for calibration
}

float ADCprocessing::processVtoN(float voltage, float units_per_V) {
  if (units_per_V < 0) return (voltage - m_Voffset) * m_units_per_V;
  return (voltage - m_Voffset) * units_per_V;
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
  m_units_per_V = objectForce / (taredVoltage);
  ESP_LOGI("ADCprocessing", "Calibration complete: Units_per_V = %.3f N/V", m_units_per_V);
}

void ADCprocessing::setScale(float scale) {
  m_units_per_V = scale;
  ESP_LOGI("ADCprocessing", "Cell scale set to: %.3f N/V", m_units_per_V);
}