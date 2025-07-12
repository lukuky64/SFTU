#include "PCA6408A.hpp"

uint8_t current_output_state = 0b00000000; // Initialize to all LOW

PCA6408A::PCA6408A(uint8_t addr, TwoWire &wire_)
{
  _i2caddr = addr;
  _I2C_BUS = &wire_; // Use the provided I2C bus instance
  _I2C_BUS->begin();
}

/**
 @brief Configure Device
*/
void PCA6408A::Initialization(uint8_t config)
{
  // writeI2c
  _config = config; // Store the configuration
  i2cWriteByte(_i2caddr, PCA6408A_CONFIGURATION_REG, _config, *_I2C_BUS);

  // Initialize all outputs to HIGH (remember it's *active LOW*)
  setDigital(PCA6408A_ALL_INPUT, OUTPUT_OPEN);
}

void PCA6408A::setMode(uint8_t port, bool input_mode)
{
  // writeI2c

  if (input_mode)
  {
    _config |= port; // Set the port to input mode
  }
  else
  {
    _config &= ~port; // Set the port to output mode
  }

  i2cWriteByte(_i2caddr, PCA6408A_CONFIGURATION_REG, _config, *_I2C_BUS);
}

/**
 @brief set Port to Digital
 @param [in] port Port
 @param [in] output output
*/
void PCA6408A::setDigital(uint8_t port, uint8_t output)
{
  if (output == OUTPUT_HIGH)
  {
    setMode(port, false);
    current_output_state |= port;
  }
  else if (output == OUTPUT_LOW)
  {
    setMode(port, false);
    current_output_state &= ~port;
  }
  else if (output == OUTPUT_OPEN)
  {
    setMode(port, true); // Set the port to input mode / high impedance
    return;              // No need to write to output register
  }
  i2cWriteByte(_i2caddr, PCA6408A_OUTPUT_REG, current_output_state, *_I2C_BUS);
}

/**
 @brief All Port to LOW
*/
void PCA6408A::setAllClear()
{
  current_output_state = 0b00000000; // Set all outputs to LOW
  i2cWriteByte(_i2caddr, PCA6408A_OUTPUT_REG, current_output_state, *_I2C_BUS);
}

/**
 @brief set Port to GPIO
 @param [in] output output (8-bit value representing all pins)
*/
void PCA6408A::setGPIO(uint8_t output)
{
  // Update the current state tracking variable
  current_output_state = output;
  // Write to the output register
  i2cWriteByte(_i2caddr, PCA6408A_OUTPUT_REG, output, *_I2C_BUS);
}

/**
 @brief get Port to Digital
 @param [in] port Port (use PCA6408A_IO0 to PCA6408A_IO7)
 @return uint8_t output (HIGH or LOW)
*/
uint8_t PCA6408A::getDigital(uint8_t port)
{
  // Read the current state of the input register
  uint8_t input_state = i2cReadByte(_i2caddr, PCA6408A_INPUT_REG, *_I2C_BUS);

  // Return HIGH if the port bit is set, otherwise LOW
  return (input_state & port) ? HIGH : LOW;
}