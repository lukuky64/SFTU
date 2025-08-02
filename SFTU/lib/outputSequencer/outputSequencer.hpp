#pragma once

#include <Arduino.h>

#include <array>
#include <map>
#include <string>
#include <vector>

#include "actuation.hpp"

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
};