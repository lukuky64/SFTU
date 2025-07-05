#include <Arduino.h>

// #include "control.hpp"
// #include "esp_log.h"

// Control* control = nullptr;

// void setup() {
//   delay(1000);
//   ESP_LOGI("Main", "Starting setup...");
//   control = new Control();
//   control->setup();
//   control->begin();
// }

// void loop() {
//   vTaskDelay(pdMS_TO_TICKS(10000));  // random delay to allow tasks to run
// }

// #include "HX711.h"

// // HX711 circuit wiring
// const int LOADCELL_DOUT_PIN = 39;  // AUX 1
// const int LOADCELL_SCK_PIN = 40;   // AUX 2

// HX711 scale;

// void setup() {
//   Serial.begin(38400);
//   Serial.println("HX711 Demo");

//   Serial.println("Initializing the scale");

//   // Initialize library with data output pin, clock input pin and gain
//   factor.
//   // Channel selection is made by passing the appropriate gain:
//   // - With a gain factor of 64 or 128, channel A is selected
//   // - With a gain factor of 32, channel B is selected
//   // By omitting the gain factor parameter, the library
//   // default "128" (Channel A) is used here.
//   scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

//   Serial.println("Before setting up the scale:");
//   Serial.print("read: \t\t");
//   Serial.println(scale.read());  // print a raw reading from the ADC

//   Serial.print("read average: \t\t");
//   Serial.println(
//       scale.read_average(20));  // print the average of 20 readings from the
//       ADC

//   Serial.print("get value: \t\t");
//   Serial.println(
//       scale.get_value(5));  // print the average of 5 readings from the ADC
//                             // minus the tare weight (not set yet)

//   Serial.print("get units: \t\t");
//   Serial.println(
//       scale.get_units(5),
//       1);  // print the average of 5 readings from the ADC minus tare weight
//            // (not set) divided by the SCALE parameter (not set yet)

//   scale.set_scale(2280.f);  // this value is obtained by calibrating the
//   scale
//                             // with known weights; see the README for details
//   scale.tare();             // reset the scale to 0

//   Serial.println("After setting up the scale:");

//   Serial.print("read: \t\t");
//   Serial.println(scale.read());  // print a raw reading from the ADC

//   Serial.print("read average: \t\t");
//   Serial.println(
//       scale.read_average(20));  // print the average of 20 readings from the
//       ADC

//   Serial.print("get value: \t\t");
//   Serial.println(
//       scale.get_value(5));  // print the average of 5 readings from the ADC
//                             // minus the tare weight, set with tare()

//   Serial.print("get units: \t\t");
//   Serial.println(
//       scale.get_units(5),
//       1);  // print the average of 5 readings from the ADC minus tare weight,
//            // divided by the SCALE parameter set with set_scale

//   Serial.println("Readings:");
// }

// void loop() {
//   Serial.print("one reading:\t");
//   Serial.print(scale.get_units(), 1);
//   Serial.print("\t| average:\t");
//   Serial.println(scale.get_units(8), 1);

//   delay(10);
// }

#include <Adafruit_ADS1X15.h>

Adafruit_ADS1115 ads; /* Use this for the 16-bit version */
// Adafruit_ADS1015 ads;     /* Use this for the 12-bit version */

void setup(void) {
  Serial.begin(115200);
  Serial.println("Hello!");

  Serial.println("Getting differential reading from AIN0 (P) and AIN1 (N)");
  Serial.println(
      "ADC Range: +/- 6.144V (1 bit = 3mV/ADS1015, 0.1875mV/ADS1115)");

  // The ADC input range (or gain) can be changed via the following
  // functions, but be careful never to exceed VDD +0.3V max, or to
  // exceed the upper and lower limits if you adjust the input range!
  // Setting these values incorrectly may destroy your ADC!
  //                                                                ADS1015
  //                                                                ADS1115
  //                                                                -------
  //                                                                -------
  ads.setGain(GAIN_FOUR);  // 2/3x gain +/- 6.144V  1 bit = 3mV 0.1875mV
  // (default) ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit =
  // 2mV      0.125mV ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V 1
  // bit = 1mV      0.0625mV ads.setGain(GAIN_FOUR);       // 4x gain
  // +/ -1.024V
  // 1 bit = 0.5mV    0.03125mV ads.setGain(GAIN_EIGHT);      // 8x gain +/-
  // 0.512V  1 bit = 0.25mV   0.015625mV ads.setGain(GAIN_SIXTEEN);    //
  // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV

  uint8_t i2c_addr = 0x48;

  // Initialize I2C with specific pins (SDA=1, SCL=0)
  Wire.setPins(1, 0);  // Set SDA and SCL pins for I2C

  if (!ads.begin(i2c_addr, &Wire)) {
    Serial.println("Failed to initialize ADS.");
    while (1);
  }

  Serial.println("ADS1115 initialized successfully!");

  ads.startADCReading(ADS1X15_REG_CONFIG_MUX_DIFF_0_1, /*continuous=*/false);
}

void loop(void) {
  // If we don't have new data, skip this iteration.
  if (!ads.conversionComplete()) {
    Serial.println("Conversion not complete, skipping iteration.");
    return;
  }

  int16_t results = ads.getLastConversionResults();

  Serial.print("Differential: ");
  Serial.print(results);
  Serial.print("(");
  Serial.print(ads.computeVolts(results));
  Serial.println("V)");

  // Start another conversion.
  ads.startADCReading(ADS1X15_REG_CONFIG_MUX_DIFF_0_1, /*continuous=*/false);

  delay(50);
}

// #include <Wire.h>

// #include "../Utils/I2C/I2C.hpp"

// TwoWire i2c(0);  // Create an instance of TwoWire for I2C bus 0

// void setup() {
//   Serial.begin(115200);
//   Serial.println("I2C Scanner");

//   // Initialize I2C with SDA=21, SCL=22 (ESP32 default I2C pins), 100kHz
//   // frequency
//   i2cBegin(1, 0, 100000, i2c);

//   Serial.println("Scanning for I2C devices...");
// }

// void loop() {
//   i2cScan(i2c);
//   delay(5000);  // Scan every 5 seconds instead of every second
// }