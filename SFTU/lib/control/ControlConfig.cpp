

#include "ControlConfig.hpp"

#include <ArduinoJson.h>

#include <string>

#include "../SD_Talker/SD_Talker.hpp"

ControlConfig::ControlConfig() : rf_frequency(DEFAULT_RF_FREQUENCY), sampling_rate(DEFAULT_SAMPLING_RATE), mode(DEFAULT_MODE) {
  for (int i = 0; i < 4; ++i) {
    adc1_channels[i] = default_adc1_channel(i);
    adc2_channels[i] = default_adc2_channel(i);
  }
}

bool ControlConfig::loadFromSD(SD_Talker& sd, const char* path) {
  if (!sd.checkFileOpen()) {
    File file = SD.open(path, FILE_READ);
    if (!file) return false;
    StaticJsonDocument<2048> doc;
    DeserializationError err = deserializeJson(doc, file);
    file.close();
    if (err) return false;
    // adc1
    JsonArray chArr1 = doc["adc1"]["channels"];
    int i = 0;
    for (JsonObject chObj : chArr1) {
      if (i >= 4) break;
      adc1_channels[i].mode = chObj["mode"].as<const char*>();
      adc1_channels[i].inputs.clear();
      for (JsonVariant v : chObj["inputs"].as<JsonArray>()) {
        adc1_channels[i].inputs.push_back(v.as<int>());
      }
      adc1_channels[i].scale_factor = chObj["scale_factor"] | 1.0f;
      adc1_channels[i].tare_bias.auto_tare = false;
      adc1_channels[i].tare_bias.value = 0.0f;
      if (chObj["tare_bias"].containsKey("auto")) {
        adc1_channels[i].tare_bias.auto_tare = chObj["tare_bias"]["auto"];
      } else if (chObj["tare_bias"].containsKey("value")) {
        adc1_channels[i].tare_bias.value = chObj["tare_bias"]["value"];
      }
      ++i;
    }
    // adc2
    JsonArray chArr2 = doc["adc2"]["channels"];
    i = 0;
    for (JsonObject chObj : chArr2) {
      if (i >= 4) break;
      adc2_channels[i].mode = chObj["mode"].as<const char*>();
      adc2_channels[i].inputs.clear();
      for (JsonVariant v : chObj["inputs"].as<JsonArray>()) {
        adc2_channels[i].inputs.push_back(v.as<int>());
      }
      adc2_channels[i].scale_factor = chObj["scale_factor"] | 1.0f;
      adc2_channels[i].tare_bias.auto_tare = false;
      adc2_channels[i].tare_bias.value = 0.0f;
      if (chObj["tare_bias"].containsKey("auto")) {
        adc2_channels[i].tare_bias.auto_tare = chObj["tare_bias"]["auto"];
      } else if (chObj["tare_bias"].containsKey("value")) {
        adc2_channels[i].tare_bias.value = chObj["tare_bias"]["value"];
      }
      ++i;
    }
    rf_frequency = doc["rf_frequency"] | rf_frequency;
    sampling_rate = doc["sampling_rate"] | sampling_rate;
    mode = doc["mode"] | mode;
    return true;
  } else {
    return false;
  }
}

bool ControlConfig::saveToSD(SD_Talker& sd, const char* path) const {
  if (!sd.checkStatus()) return false;
  File file = SD.open(path, FILE_WRITE);
  if (!file) return false;
  StaticJsonDocument<2048> doc;
  // adc1
  JsonArray chArr1 = doc["adc1"]["channels"].to<JsonArray>();
  for (int i = 0; i < 4; ++i) {
    JsonObject chObj = chArr1.createNestedObject();
    chObj["mode"] = adc1_channels[i].mode.c_str();
    JsonArray inArr = chObj["inputs"].to<JsonArray>();
    for (int v : adc1_channels[i].inputs) inArr.add(v);
    chObj["scale_factor"] = adc1_channels[i].scale_factor;
    JsonObject tbObj = chObj["tare_bias"].to<JsonObject>();
    if (adc1_channels[i].tare_bias.auto_tare) {
      tbObj["auto"] = true;
    } else {
      tbObj["value"] = adc1_channels[i].tare_bias.value;
    }
  }
  // adc2
  JsonArray chArr2 = doc["adc2"]["channels"].to<JsonArray>();
  for (int i = 0; i < 4; ++i) {
    JsonObject chObj = chArr2.createNestedObject();
    chObj["mode"] = adc2_channels[i].mode.c_str();
    JsonArray inArr = chObj["inputs"].to<JsonArray>();
    for (int v : adc2_channels[i].inputs) inArr.add(v);
    chObj["scale_factor"] = adc2_channels[i].scale_factor;
    JsonObject tbObj = chObj["tare_bias"].to<JsonObject>();
    if (adc2_channels[i].tare_bias.auto_tare) {
      tbObj["auto"] = true;
    } else {
      tbObj["value"] = adc2_channels[i].tare_bias.value;
    }
  }
  doc["rf_frequency"] = rf_frequency;
  doc["sampling_rate"] = sampling_rate;
  doc["mode"] = mode;
  serializeJsonPretty(doc, file);
  file.close();
  return true;
}
