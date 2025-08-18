#include "outputSequencer.hpp"

outputSequencer::outputSequencer(Actuation *actuation) : m_actuation(actuation), lastSequenceStart(0), m_firstRun(true) {
  // Create command queue and background task
  m_cmdQueue = xQueueCreate(4, sizeof(SeqCommand));
  if (m_cmdQueue) {
    xTaskCreate([](void *param) { static_cast<outputSequencer *>(param)->taskLoop(); }, "SeqTask", 4096, this, 2, &m_taskHandle);
  }
}

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
  // Non-blocking: enqueue a start command for the background task
  if (!m_cmdQueue) return;
  SeqCommand cmd{CmdType::Start, uid};
  xQueueSend(m_cmdQueue, &cmd, 0);
}

void outputSequencer::stopSequence() {
  // Non-blocking: signal the sequencer task to stop immediately
  seqRunning = false;  // allow immediate visibility
  if (!m_cmdQueue) {
    // Fallback: clear outputs if task/queue not ready
    m_actuation->setAllClear();
    return;
  }
  SeqCommand cmd{CmdType::Stop, 0};
  xQueueSend(m_cmdQueue, &cmd, 0);
}

void outputSequencer::taskLoop() {
  TickType_t lastWake = xTaskGetTickCount();
  const TickType_t tick = pdMS_TO_TICKS(1);

  // current sequence context
  uint16_t activeUid = 0;
  size_t blockIndex = 0;
  unsigned long blockStartMs = 0;
  sequence empty;
  const sequence *activeSeq = &empty;

  for (;;) {
    // Process commands quickly
    SeqCommand cmd;
    while (xQueueReceive(m_cmdQueue, &cmd, 0) == pdTRUE) {
      if (cmd.type == CmdType::Stop) {
        seqRunning = false;
        m_actuation->setAllClear();
        activeSeq = &empty;
      } else if (cmd.type == CmdType::Start) {
        // Enforce start guard
        if ((millis() - lastSequenceStart > NEXT_SEQ_PERIOD) || m_firstRun) {
          auto it = allSequences.find(cmd.uid);
          if (it != allSequences.end()) {
            activeUid = cmd.uid;
            activeSeq = &it->second;
            blockIndex = 0;
            seqRunning = true;
            m_firstRun = false;
            lastSequenceStart = millis();
            blockStartMs = 0;  // start immediately
          } else {
            Serial.printf("Sequence with UID %d not found.\n", cmd.uid);
          }
        }
      }
    }

    // Execute current sequence if running
    if (seqRunning && activeSeq && !activeSeq->empty()) {
      if (blockIndex >= activeSeq->size()) {
        // finished
        seqRunning = false;
        m_actuation->setAllClear();  // automatically turn off all outputs at end of sequence
        activeSeq = &empty;
      } else {
        const auto &block = (*activeSeq)[blockIndex];
        if (blockStartMs == 0) {
          // start this block
          m_actuation->setDigital(PCA6408A_outputPins[block.channel], (block.state ? OUTPUT_LOW : OUTPUT_OPEN));
          blockStartMs = millis();
        }
        // Check for duration expiry or external stop
        if (!seqRunning || (millis() - blockStartMs >= block.durationMS)) {
          blockIndex++;
          blockStartMs = 0;
        }
      }
    }

    vTaskDelayUntil(&lastWake, tick);
  }
}

void outputSequencer::stopFromISR() {
  // ISR-safe way to request stop: enqueue a Stop command from ISR
  if (m_cmdQueue) {
    SeqCommand cmd{CmdType::Stop, 0};
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(m_cmdQueue, &cmd, &xHigherPriorityTaskWoken);
#if portCHECK_IF_IN_ISR == 1
    if (xHigherPriorityTaskWoken) portYIELD_FROM_ISR();
#else
    (void)xHigherPriorityTaskWoken;
#endif
  } else {
    // Fallback (not ideal in ISR): set flag, actual clearing will happen in task context later
    seqRunning = false;
  }
}