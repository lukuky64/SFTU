#pragma once

// Official Heltec ESP32-C3 + SX1262 pin mapping from datasheet
#define RF_DIO 3   // GPIO3, ADC1_CH3, connected to SX1262_DIO1
#define RF_BUSY 4  // GPIO4, ADC1_CH4, FSPIHD, MTMS, connected to SX1262_BUSY
#define RF_RST 5   // GPIO5, ADC2_CH0, FSPIWP, MTDI, connected to SX1262_RST
#define SPI_MISO_RF 6  // GPIO6, FSPICLK, MTCK, connected to SX1262_MISO
#define SPI_MOSI_RF 7  // GPIO7, FSPID, MTDO, connected to SX1262_MOSI
#define SPI_CS_RF 8    // GPIO8, connected to SX1262_NSS
#define SPI_CLK_RF 10  // GPIO10, FSPICS0, connected to SX1262_SCK

#define INDICATOR_LED1 2