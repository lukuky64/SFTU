
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
    ch.mode = (i < 2) ? "differential" : "single_ended";
    ch.inputs = (i < 2) ? std::vector<int>{i * 2, i * 2 + 1} : std::vector<int>{i};
    ch.scale_factor = 1.0f;
    ch.tare_bias.auto_tare = true;
    ch.tare_bias.value = 0.0f;
    return ch;
  }
  static ChannelConfig default_adc2_channel(int i) {
    ChannelConfig ch;
    ch.mode = (i == 0) ? "differential" : "single_ended";
    ch.inputs = (i == 0) ? std::vector<int>{0, 1} : std::vector<int>{i};
    ch.scale_factor = 1.0f;
    ch.tare_bias.auto_tare = true;
    ch.tare_bias.value = 0.0f;
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
};
