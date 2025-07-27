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

// Constants

#define VBATT_SCALE 4.032f  // Voltage divider scale factor for battery voltage measurement
#define CELL_SCALE 1494.0f  // Scale factor for force measurement. 5kg Cell <50.638434f>, 300kg Cell <1494.0f>

// The Pressure transducers i've tested seem to be uni-directional Vout = 0.1*Vbias at atmospheric pressure,
// So we can assume a sensitivity given the range is from 0.1*Vbias to 0.9*Vbias, and 0-X PSI.

#define EXCITATION_BIAS 4.096f

#define PT_1600_SCALE 1600.0f / (0.8 * 4.096f)  // Pressure transducer scale factor. 1600 PSI <>
#define PT_150_SCALE 150.0f / (0.8 * 4.096f)    // 0.8 is the span (0.1 to 0.9), 150 is the full scale of the sensor. 4.096f is the Vbias

#define ADC_SPS 125  // about 333Hz max in single shot mode. 860Hz in continuous mode

// 5kg Cell (50.638434 N/V), 300kg Cell (?)