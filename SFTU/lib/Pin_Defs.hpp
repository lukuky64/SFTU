#pragma once

// #define PROTOBOARD

// UART
#define MCU_RX 44
#define MCU_TX 43

// I2C
#define I2C1_SCL 0
#define I2C1_SDA 1

#define I2C2_SCL 2
#define I2C2_SDA 3

// AUX
#define AUX_1 39
#define AUX_2 40

// RF LoRa
#define RF_RST 8
#define RF_DIO 9
#define SPI_CS_RF 10
#define SPI_MOSI_RF 11
#define SPI_CLK_RF 12
#define SPI_MISO_RF 13

#ifdef PROTOBOARD
#define SD_CD 3
#else
#define SD_CD 48
#endif

#define SPI_CLK_SD 35
#define SPI_MOSI_SD 36
#define SPI_MISO_SD 37
#define SPI_CS_SD 38

// BMS
#define VOLTAGE_SENSE 6

// Indicators
#define INDICATOR_LED1 15
#define INDICATOR_LED2 4
#define INDICATOR_LED3 5
#define INDICATOR_LED4 14

#define IGNITER_CONTROL 16
#define IGNITER_SENSE 4
#define IGNITER_ARMED 18

#define EXT_BTN1 17
#define EXT_BTN2 7