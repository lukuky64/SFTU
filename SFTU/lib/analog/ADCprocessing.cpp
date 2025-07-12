#include "ADCprocessing.hpp"

float VtoN_cell = 50'638.434f;
float Voffset = 0.0f;        // Offset voltage for calibration
const float gravity = 9.81f; // Acceleration due to gravity in m/s^2

float processVtoN(float voltage) { return (voltage - Voffset) * VtoN_cell; }

void tareVolts(float voltage) { Voffset = voltage; }

// use commander to call this function
void calibrate(float objectMass, float voltage)
{
  ESP_LOGI("ADCprocessing", "rawVoltage: %.3f V, Voffset: %.3f V", voltage,
           Voffset);

  float taredVoltage = voltage - Voffset;
  ESP_LOGI("ADCprocessing",
           "Calibrating with object mass: %.3f kg, tared voltage: %.3f V",
           objectMass, taredVoltage);
  float objectForce = objectMass * gravity;
  VtoN_cell = objectForce / (voltage - Voffset);
  ESP_LOGI("ADCprocessing", "Calibration complete: VtoN_cell = %.3f N/V",
           VtoN_cell);
}
