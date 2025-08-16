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
  // Internal command types for the sequencer task
  enum class CmdType : uint8_t { Start = 0, Stop = 1 };
  struct SeqCommand {
    CmdType type;
    uint16_t uid;
  };

  sequences allSequences;
  Actuation *m_actuation;
  volatile bool seqRunning = false;

  unsigned long lastSequenceStart;
  bool m_firstRun;

  // Background task and command queue
  TaskHandle_t m_taskHandle = nullptr;
  QueueHandle_t m_cmdQueue = nullptr;

  // Sequencer task loop (runs in its own FreeRTOS task)
  void taskLoop();
};