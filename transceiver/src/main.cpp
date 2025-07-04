#include <Arduino.h>

#include "control.hpp"

Control* control = nullptr;

void setup() {
  delay(1000);
  ESP_LOGI("Main", "Starting setup...");
  control = new Control();
  control->setup();
  control->begin();
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(10000));  // random delay to allow tasks to run
}

/*
  RadioLib SX126x Receive after Channel Activity Detection Example

  This example uses SX1262 to scan the current LoRa
  channel and detect ongoing LoRa transmissions.
  Unlike SX127x CAD, SX126x can detect any part
  of LoRa transmission, not just the preamble.
  If a packet is detected, the module will switch
  to receive mode and receive the packet.

  Other modules from SX126x family can also be used.

  For default module settings, see the wiki page
  https://github.com/jgromes/RadioLib/wiki/Default-configuration#sx126x---lora-modem

  For full API reference, see the GitHub Pages
  https://jgromes.github.io/RadioLib/
*/

// // include the library
// #include <RadioLib.h>

// // SX1262 has the following connections:
// // NSS pin:   10
// // DIO1 pin:  2
// // NRST pin:  3
// // BUSY pin:  9
// SX1262 radio = new Module(8, 3, 5, 4);

// // or detect the pinout automatically using RadioBoards
// // https://github.com/radiolib-org/RadioBoards
// /*
// #define RADIO_BOARD_AUTO
// #include <RadioBoards.h>
// Radio radio = new RadioModule();
// */

// // whether we are receiving, or scanning
// bool receiving = false;

// // flag to indicate that a packet was detected or CAD timed out
// volatile bool scanFlag = false;

// // this function is called when a complete packet
// // is received by the module
// // IMPORTANT: this function MUST be 'void' type
// //            and MUST NOT have any arguments!
// #if defined(ESP8266) || defined(ESP32)
// ICACHE_RAM_ATTR
// #endif
// void setFlag(void) {
//   // something happened, set the flag
//   scanFlag = true;
// }

// void setup() {
//   Serial.begin(115200);
//   delay(1000);

//   // initialize SX1262 with default settings
//   Serial.print(F("[SX1262] Initializing ... "));
//   SPI.begin(10, 6, 7, 8);  // CLK, MISO, MOSI, csPin
//   int state = radio.begin(915.0, 125, 12, 5, 0x34, 22, 20);
//   if (state == RADIOLIB_ERR_NONE) {
//     Serial.println(F("success!"));
//   } else {
//     Serial.print(F("failed, code "));
//     Serial.println(state);
//     while (true) {
//       delay(10);
//     }
//   }

//   // set the function that will be called
//   // when LoRa packet or timeout is detected
//   radio.setDio1Action(setFlag);

//   // start scanning the channel
//   Serial.print(F("[SX1262] Starting scan for LoRa preamble ... "));
//   state = radio.startChannelScan();
//   if (state == RADIOLIB_ERR_NONE) {
//     Serial.println(F("success!"));
//   } else {
//     Serial.print(F("failed, code "));
//     Serial.println(state);
//   }
// }

// void loop() {
//   // check if the flag is set
//   if (scanFlag) {
//     int state = RADIOLIB_ERR_NONE;

//     // reset flag
//     scanFlag = false;

//     // check ongoing reception
//     if (receiving) {
//       // DIO triggered while reception is ongoing
//       // that means we got a packet

//       // you can read received data as an Arduino String
//       String str;
//       state = radio.readData(str);

//       // you can also read received data as byte array
//       /*
//         byte byteArr[8];
//         state = radio.readData(byteArr, 8);
//       */

//       if (state == RADIOLIB_ERR_NONE) {
//         // packet was successfully received
//         Serial.println(F("[SX1262] Received packet!"));

//         // print data of the packet
//         Serial.print(F("[SX1262] Data:\t\t"));
//         Serial.println(str);

//         // print RSSI (Received Signal Strength Indicator)
//         Serial.print(F("[SX1262] RSSI:\t\t"));
//         Serial.print(radio.getRSSI());
//         Serial.println(F(" dBm"));

//         // print SNR (Signal-to-Noise Ratio)
//         Serial.print(F("[SX1262] SNR:\t\t"));
//         Serial.print(radio.getSNR());
//         Serial.println(F(" dB"));

//         // print frequency error
//         Serial.print(F("[SX1262] Frequency error:\t"));
//         Serial.print(radio.getFrequencyError());
//         Serial.println(F(" Hz"));

//       } else {
//         // some other error occurred
//         Serial.print(F("[SX1262] Failed, code "));
//         Serial.println(state);
//       }

//       // reception is done now
//       receiving = false;

//     } else {
//       // check CAD result
//       state = radio.getChannelScanResult();

//       if (state == RADIOLIB_LORA_DETECTED) {
//         // LoRa packet was detected
//         Serial.print(F("[SX1262] Packet detected, starting reception ... "));
//         state = radio.startReceive();
//         if (state == RADIOLIB_ERR_NONE) {
//           Serial.println(F("success!"));
//         } else {
//           Serial.print(F("failed, code "));
//           Serial.println(state);
//         }

//         // set the flag for ongoing reception
//         receiving = true;

//       } else if (state == RADIOLIB_CHANNEL_FREE) {
//         // channel is free
//         // Serial.println(F("[SX1262] Channel is free!"));

//       } else {
//         // some other error occurred
//         Serial.print(F("[SX1262] Failed, code "));
//         Serial.println(state);
//       }
//     }

//     // if we're not receiving, start scanning again
//     if (!receiving) {
//       // Serial.print(F("[SX1262] Starting scan for LoRa preamble ... "));
//       state = radio.startChannelScan();
//       if (state == RADIOLIB_ERR_NONE) {
//         // Serial.println(F("success!"));
//       } else {
//         Serial.print(F("failed, code "));
//         Serial.println(state);
//       }
//     }
//   }
// }
