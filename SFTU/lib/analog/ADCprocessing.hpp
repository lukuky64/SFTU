#pragma once
#include <Arduino.h>

extern float VtoN_cell;
extern float Voffset;        // Offset voltage for calibration
extern const float gravity;  // Acceleration due to gravity in m/s^2

float processVtoN(float voltage);

void tareVolts(float voltage);

void calibrate(float objectMass, float voltage);