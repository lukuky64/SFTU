#include "loadCellProcessing.hpp"

loadCellProcessing::loadCellProcessing() {
  m_units_per_V = 0.0f;  // Initialize units per volt
  m_Voffset = 0.0f;      // Offset voltage for calibration
}

float loadCellProcessing::processVtoN(float voltage, float units_per_V) {
  if (units_per_V < 0) return (voltage - m_Voffset) * m_units_per_V;
  return (voltage - m_Voffset) * units_per_V;
}

void loadCellProcessing::tareVolts(float voltage) {
  m_Voffset = voltage;
  //
}

// use commander to call this function
void loadCellProcessing::calibrate(float objectMass, float voltage) {
  ESP_LOGI("loadCellProcessing", "rawVoltage: %.3f V, Voffset: %.3f V", voltage, m_Voffset);

  float taredVoltage = voltage - m_Voffset;
  ESP_LOGI("loadCellProcessing", "Calibrating with object mass: %.3f kg, tared voltage: %.3f V", objectMass, taredVoltage);
  float objectForce = objectMass * m_gravity;
  m_units_per_V = objectForce / (taredVoltage);
  ESP_LOGI("loadCellProcessing", "Calibration complete: Units_per_V = %.3f N/V", m_units_per_V);
}

void loadCellProcessing::setScale(float scale) {
  m_units_per_V = scale;
  ESP_LOGI("loadCellProcessing", "Cell scale set to: %.3f N/V", m_units_per_V);
}