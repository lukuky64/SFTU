#pragma once

#include <Arduino.h>

#include <array>
#include <map>
#include <string>
#include <vector>

#include "actuation.hpp"

#define NEXT_SEQ_PERIOD 30'000  // ms. Time to wait before starting a new sequence. Helps wiith repeated control command through LoRa

struct sequenceBlock {
  uint8_t channel;      // 1-8
  bool state;           // true for ON, false for OFF
  uint16_t durationMS;  // Duration before next sequence block
};

using sequence = std::vector<sequenceBlock>;
using sequences = std::map<uint16_t, sequence>;
class outputSequencer {
 public:
  outputSequencer(Actuation *actuation);
  void createSequence(String sequenceString, uint16_t uid = 0);
  void startSequence(uint16_t uid = 0);
  void stopSequence();

 private:
  sequences allSequences;
  Actuation *m_actuation;
  volatile bool seqRunning = false;

  unsigned long lastSequenceStart = 0;
};