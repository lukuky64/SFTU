#include "PTProcessing.hpp"

PTProcessing::PTProcessing() {
  m_units_per_V = 0.0f;  // Initialize units per volt
  m_Voffset = 0.0f;      // Offset voltage for calibration
}

float PTProcessing::processVtoPSI(float voltage, float units_per_V) {
  if (units_per_V < 0) return (voltage - m_Voffset) * m_units_per_V;
  return (voltage - m_Voffset) * units_per_V;
}

void PTProcessing::tareVolts(float voltage) {
  m_Voffset = voltage;
  //
}

// use commander to call this function
// void PTProcessing::calibrate(float objectMass, float voltage) {
//   ESP_LOGI("PTProcessing", "rawVoltage: %.3f V, Voffset: %.3f V", voltage, m_Voffset);

//   float taredVoltage = voltage - m_Voffset;
//   ESP_LOGI("PTProcessing", "Calibrating with object mass: %.3f kg, tared voltage: %.3f V", objectMass, taredVoltage);
//   float objectForce = objectMass * m_gravity;
//   m_units_per_V = objectForce / (taredVoltage);
//   ESP_LOGI("PTProcessing", "Calibration complete: Units_per_V = %.3f N/V", m_units_per_V);
// }

void PTProcessing::setScale(float scale) {
  m_units_per_V = scale;
  ESP_LOGI("PTProcessing", "Cell scale set to: %.3f N/V", m_units_per_V);
}