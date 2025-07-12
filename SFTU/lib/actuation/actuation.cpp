
#include "actuation.hpp"

Actuation::Actuation(uint8_t addr1, uint8_t addr2, TwoWire &wire)
    : _gpioExpanderOutput(addr1, wire), _gpioExpanderInput(addr2, wire) {}

void Actuation::init()
{
  uint8_t configOutputs = PCA6408A_ALL_OUTPUT;
  _gpioExpanderOutput.Initialization(configOutputs);

  uint8_t configInputs = PCA6408A_ALL_INPUT;
  _gpioExpanderInput.Initialization(configInputs);
}

void Actuation::setDigital(uint8_t port, uint8_t output)
{
  _gpioExpanderOutput.setDigital(port, output);
}
uint8_t Actuation::getDigital(uint8_t port)
{
  // Read the input state from the GPIO expander
  return _gpioExpanderInput.getDigital(port);
}

void Actuation::setAllClear() { _gpioExpanderOutput.setAllClear(); }

void Actuation::setGPIO(uint8_t output) { _gpioExpanderOutput.setGPIO(output); }

// Global variable to track the current output state
