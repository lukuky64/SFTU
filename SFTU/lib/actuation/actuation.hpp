#pragma once

#include <Arduino.h>

#include <gpio_expander/PCA6408A.hpp>

class Actuation
{
public:
  Actuation(uint8_t addr1, uint8_t addr2, TwoWire &wire);
  void init();
  void setDigital(uint8_t port, uint8_t output);
  void setAllClear();
  void setGPIO(uint8_t output);
  uint8_t getDigital(uint8_t port);

private:
  PCA6408A _gpioExpanderOutput;
  PCA6408A _gpioExpanderInput;
};
