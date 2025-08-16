
#pragma once

#include <stdint.h>

#include <array>
#include <string>
#include <vector>

class SD_Talker;

struct TareBias {
  bool auto_tare = false;
  float value = 0.0f;
};

struct ChannelConfig {
  std::string name;
  std::string units;
  std::string mode;
  std::vector<int> inputs;
  float scale_factor = 1.0f;
  TareBias tare_bias;
  int mux = -1;
};

class ControlConfig {
 public:
  static constexpr uint32_t DEFAULT_RF_FREQUENCY = 915000000;
  static constexpr int DEFAULT_SAMPLING_RATE = 125;
  static constexpr int DEFAULT_MODE = 0;

  static ChannelConfig default_adc1_channel(int i) {
    ChannelConfig ch;
    if (i == 0) {
      ch.name = "Load Cell 1";
      ch.units = "N";
      ch.mode = "differential";
      ch.inputs = {0, 1};
      ch.scale_factor = 1494.0f;
      ch.tare_bias.auto_tare = true;
      ch.tare_bias.value = 0.0f;
    } else if (i == 1) {
      ch.name = "Load Cell 2";
      ch.units = "N";
      ch.mode = "differential";
      ch.inputs = {2, 3};
      ch.scale_factor = 1494.0f;
      ch.tare_bias.auto_tare = true;
      ch.tare_bias.value = 0.0f;
    } else {
      ch.name = "";
      ch.units = "";
      ch.mode = "unused";
      ch.inputs = {};
      ch.scale_factor = 1.0f;
      ch.tare_bias.auto_tare = false;
      ch.tare_bias.value = 0.0f;
    }
    return ch;
  }
  static ChannelConfig default_adc2_channel(int i) {
    ChannelConfig ch;
    if (i == 0) {
      ch.name = "Load Cell 3";
      ch.units = "N";
      ch.mode = "differential";
      ch.inputs = {0, 1};
      ch.scale_factor = 1494.0f;
      ch.tare_bias.auto_tare = true;
      ch.tare_bias.value = 0.0f;
    } else if (i == 1) {
      ch.name = "Pressure Transducer 1";
      ch.units = "psi";
      ch.mode = "single_ended";
      ch.inputs = {2};
      ch.scale_factor = 488.28f;
      ch.tare_bias.auto_tare = false;
      ch.tare_bias.value = 0.4096f;
    } else if (i == 2) {
      ch.name = "Pressure Transducer 2";
      ch.units = "psi";
      ch.mode = "single_ended";
      ch.inputs = {3};
      ch.scale_factor = 488.28f;
      ch.tare_bias.auto_tare = false;
      ch.tare_bias.value = 0.4096f;
    } else {
      ch.name = "";
      ch.units = "";
      ch.mode = "unused";
      ch.inputs = {};
      ch.scale_factor = 1.0f;
      ch.tare_bias.auto_tare = false;
      ch.tare_bias.value = 0.0f;
    }
    return ch;
  }

  std::array<ChannelConfig, 4> adc1_channels;  // 4 channels per ADC1
  std::array<ChannelConfig, 4> adc2_channels;  // 4 channels per ADC2
  uint32_t rf_frequency;                       // RF frequency in Hz
  int sampling_rate;                           // ADC sampling rate (SPS)
  int mode;                                    // Operation mode (0=normal, 1=RF off, etc)

  ControlConfig();

  bool loadFromSD(SD_Talker& sd, const char* path);
  bool saveToSD(SD_Talker& sd, const char* path) const;
  std::vector<std::string> getChannelNames() const;
  std::vector<std::string> getChannelUnits() const;
};
