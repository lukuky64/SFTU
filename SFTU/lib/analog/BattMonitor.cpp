#include "BattMonitor.hpp"

BattMonitor::BattMonitor(uint8_t sensePin, float scaleFactor)
    : m_sensePin(sensePin),
      m_scaleFactor(scaleFactor),
      m_initialised(false),
      m_batteryVoltage(0.0f) {}

void BattMonitor::init()
{
  if (!m_initialised)
  {
    pinMode(m_sensePin, INPUT);
    analogSetPinAttenuation(m_sensePin, ADC_11db); // range = 150 mV ~ 2450 mV.
    // Documentation says ESP32-S3 ADC range is 0 ~ 3100 mV
    analogReadResolution(12); // 4096 steps
    m_initialised = true;
    ESP_LOGD("BattMonitor", "Battery monitor initialized on pin %d",
             m_sensePin);
  }
}

float BattMonitor::getRawVoltage(uint8_t samples)
{
  if (!m_initialised)
    init();
  float rawV = 0.0f;
  samples = samples == 0 ? 1 : samples; // Prevent division by zero
  for (uint8_t i = 0; i < samples; i++)
  {
    uint32_t raw_mV = analogReadMilliVolts(m_sensePin);
    rawV += static_cast<float>(raw_mV) / 1000.0f; // Convert to volts
  }
  rawV /= samples;
  return rawV;
}

float BattMonitor::getScaledVoltage(uint8_t samples)
{
  float rawV = getRawVoltage(samples);
  m_batteryVoltage = rawV * m_scaleFactor; // Apply scale factor
  return m_batteryVoltage;
}