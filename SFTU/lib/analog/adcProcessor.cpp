#include "adcProcessor.hpp"

adcProcessor::adcProcessor() {
  m_units_per_V = 0.0f;  // Initialize units per volt
  m_Voffset = 0.0f;      // Offset voltage for calibration
}

float adcProcessor::processVtoUnits(float voltage, float units_per_V) {
  if (units_per_V < 0) return (voltage - m_Voffset) * m_units_per_V;
  return (voltage - m_Voffset) * units_per_V;
}

void adcProcessor::tareVolts(float voltage) {
  m_Voffset = voltage;
  //
}

// use commander to call this function
float adcProcessor::calibrate(float realUnits, float voltage) {
  ESP_LOGI(TAG, "rawVoltage: %.3f V, Voffset: %.3f V", voltage, m_Voffset);

  float taredVoltage = voltage - m_Voffset;
  ESP_LOGI(TAG, "Calibrating with real units: %.3f U, tared voltage: %.3f V", realUnits, taredVoltage);
  m_units_per_V = realUnits / (taredVoltage);
  ESP_LOGI(TAG, "Calibration complete: Units_per_V = %.3f N/V", m_units_per_V);
  return m_units_per_V;
}

void adcProcessor::setScale(float scale) {
  m_units_per_V = scale;
  ESP_LOGI(TAG, "Cell scale set to: %.3f N/V", m_units_per_V);
}