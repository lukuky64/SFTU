#include "outputSequencer.hpp"

outputSequencer::outputSequencer(Actuation *actuation) : m_actuation(actuation) {}

void outputSequencer::createSequence(String sequenceString, uint16_t uid) {
  // String will be in the format: "CHANNEL:STATE:DURATION;CHANNEL:STATE:DURATION..."

  sequence seq;

  int start = 0;
  int end = sequenceString.indexOf(';');
  while (end != -1) {
    String block = sequenceString.substring(start, end);
    int firstColon = block.indexOf(':');
    int lastColon = block.lastIndexOf(':');
    if (firstColon == -1 || lastColon == -1 || firstColon == lastColon) {
      start = end + 1;
      end = sequenceString.indexOf(';', start);
      continue;  // skip malformed block
    }
    int channel = block.substring(0, firstColon).toInt();
    bool state = block.substring(firstColon + 1, lastColon).toInt();
    uint16_t duration = block.substring(lastColon + 1).toInt();
    seq.push_back({static_cast<uint8_t>(channel), state, duration});
    start = end + 1;
    end = sequenceString.indexOf(';', start);
  }
  // Process the last block (after the last ';' or the only block if no ';')
  if (start < sequenceString.length()) {
    String block = sequenceString.substring(start);
    int firstColon = block.indexOf(':');
    int lastColon = block.lastIndexOf(':');
    if (firstColon != -1 && lastColon != -1 && firstColon != lastColon) {
      int channel = block.substring(0, firstColon).toInt();
      bool state = block.substring(firstColon + 1, lastColon).toInt();
      uint16_t duration = block.substring(lastColon + 1).toInt();
      seq.push_back({static_cast<uint8_t>(channel), state, duration});
    }
  }
  allSequences[uid] = seq;
}

void outputSequencer::startSequence(uint16_t uid) {
  auto it = allSequences.find(uid);
  if (it != allSequences.end()) {
    seqRunning = true;
    sequence &seq = it->second;

    for (const auto &block : seq) {
      if (seqRunning) {
        m_actuation->setDigital(PCA6408A_outputPins[block.channel], (block.state ? OUTPUT_LOW : OUTPUT_OPEN));

        // this will give better timing control
        unsigned long startTime = millis();
        while (millis() - startTime < block.durationMS) {
          vTaskDelay(pdMS_TO_TICKS(1));
        }
      } else {
        break;
      }
    }
  } else {
    Serial.printf("Sequence with UID %d not found.\n", uid);
  }
}

void outputSequencer::stopSequence() {
  seqRunning = false;  // Stop the sequence by setting the flag to false
  for (int i = 1; i <= 8; ++i) {
    m_actuation->setDigital(PCA6408A_outputPins[i], OUTPUT_OPEN);
  }
}