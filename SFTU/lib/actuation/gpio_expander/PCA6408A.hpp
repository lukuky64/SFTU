/**
 * @file PCA6408A.hpp
 *
 * @brief
 * This header file defines the PCA6408A class for interfacing with the PCA6408A
 * I/O expander chip using the I2C communication protocol. The PCA6408A is an
 * 8-bit I/O expander that provides GPIO (General Purpose Input/Output)
 * extension for microcontrollers over the I2C bus.
 *
 * The class includes methods to configure the device, set individual I/O pins
 * as either input or output, write digital values to output pins, and clear all
 * outputs. This library simplifies control of the PCA6408A chip with high-level
 * function calls, suitable for Arduino or compatible microcontrollers.
 *
 * Usage:
 * - Initialize the PCA6408A with `PCA6408A.Initialization()`.
 * - Set individual I/O pins with `PCA6408A.setDigital(PCA6408A_IO0, HIGH)` for
 * example.
 * - Clear all outputs with `PCA6408A.setAllClear()`.
 *
 * Example:
 *     PCA6408A pca6408a;
 *     pca6408a.Initialization();                // Initialize PCA6408A
 *     pca6408a.setDigital(PCA6408A_IO0, HIGH);  // Set IO0 to HIGH
 *     pca6408a.setAllClear();                   // Set all ports to LOW
 *
 * @author
 *     Itay Nave, Embedded Software Engineer
 * @date
 *     12/25/2023
 *
 * @copyright
 *     © 2023 Itay Nave. All rights reserved.
 *     This software is provided "as is", without warranty of any kind, express
 * or implied. Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the above copyright notice
 * and this permission notice appear in all copies.
 */

#pragma once

#include <Arduino.h>

#include "../Utils/I2C/I2C.hpp"

// Default I2C Slave Address
#define PCA6408A_SLAVE_ADDRESS_L 0x20
#define PCA6408A_SLAVE_ADDRESS_H 0x21

#define OUTPUT_OPEN 2
#define OUTPUT_HIGH 1
#define OUTPUT_LOW 0

// Register addresses
#define PCA6408A_OUTPUT_REG 0x01
#define PCA6408A_INPUT_REG 0x00
#define PCA6408A_CONFIGURATION_REG 0x03

#define PCA6408A_IO0_OUTPUT 0b00000000
#define PCA6408A_IO0_INPUT 0b00000001
#define PCA6408A_IO1_OUTPUT 0b00000000
#define PCA6408A_IO1_INPUT 0b00000010
#define PCA6408A_IO2_OUTPUT 0b00000000
#define PCA6408A_IO2_INPUT 0b00000100
#define PCA6408A_IO3_OUTPUT 0b00000000
#define PCA6408A_IO3_INPUT 0b00001000
#define PCA6408A_IO4_OUTPUT 0b00000000
#define PCA6408A_IO4_INPUT 0b00010000
#define PCA6408A_IO5_OUTPUT 0b00000000
#define PCA6408A_IO5_INPUT 0b00100000
#define PCA6408A_IO6_OUTPUT 0b00000000
#define PCA6408A_IO6_INPUT 0b01000000
#define PCA6408A_IO7_OUTPUT 0b00000000
#define PCA6408A_IO7_INPUT 0b10000000

#define PCA6408A_ALL_INPUT 0b11111111
#define PCA6408A_ALL_OUTPUT 0b00000000

// Port names
#define PCA6408A_IO0 0b00000001
#define PCA6408A_IO1 0b00000010
#define PCA6408A_IO2 0b00000100
#define PCA6408A_IO3 0b00001000
#define PCA6408A_IO4 0b00010000
#define PCA6408A_IO5 0b00100000
#define PCA6408A_IO6 0b01000000
#define PCA6408A_IO7 0b10000000

// Array of PCA6408A pin constants (indexed from 1-8)
static const uint8_t PCA6408A_outputPins[] = {
    0,            // Index 0 (unused)
    PCA6408A_IO0, // Index 1 -> Output 1
    PCA6408A_IO1, // Index 2 -> Output 2
    PCA6408A_IO2, // Index 3 -> Output 3
    PCA6408A_IO3, // Index 4 -> Output 4
    PCA6408A_IO4, // Index 5 -> Output 5
    PCA6408A_IO5, // Index 6 -> Output 6
    PCA6408A_IO6, // Index 7 -> Output 7
    PCA6408A_IO7  // Index 8 -> Output 8
};

class PCA6408A
{
public:
  PCA6408A(uint8_t addr, TwoWire &wire);
  void Initialization(uint8_t config);
  void setDigital(uint8_t port, uint8_t output);
  void setAllClear();
  void setGPIO(uint8_t output);
  uint8_t getDigital(uint8_t port);
  void setMode(uint8_t port, bool input_mode);

private:
  uint8_t _i2caddr;
  TwoWire *_I2C_BUS; // Pointer to the I2C bus instance
  uint8_t _config;
};

// Global variable to track the current output state
extern uint8_t current_output_state;