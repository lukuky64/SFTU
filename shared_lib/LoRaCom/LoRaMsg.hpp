#pragma once

#include <Arduino.h>

#define MAX_PAYLOAD_SIZE 32

#define MAX_QUEUE_SIZE 5
#define ACK_TIMEOUT_MS 500
#define TX_TIMEOUT_MS 1000
#define MAX_RETRIES 5

#pragma pack(push, 1)
struct LoRaMessage
{
  uint8_t senderID;
  uint8_t receiverID;
  uint8_t sequenceID;
  uint8_t type;
  uint8_t length;
  uint8_t payload[MAX_PAYLOAD_SIZE];
};

struct StatusPayload
{
  int8_t rssi;
  float batteryVoltage;
  uint8_t status; // e.g. enum or code
};

struct CommandPayload
{
  uint8_t commandID;
  float param;
};

struct AckPayload
{
  uint8_t acknowledgedSequenceID;
};
#pragma pack(pop)

struct QueuedMessage
{
  LoRaMessage msg;
  uint8_t retryCount = 0;
  unsigned long lastSendTime = 0;
  bool acknowledged = false;
  bool failed = false;
  bool reqAck = false;
};

enum messageType
{
  TYPE_STATUS = 0,
  TYPE_COMMAND = 1,
  TYPE_ACK = 2,
};

enum deviceStatus
{
  STATUS_OK = 0,
  STATUS_ERROR = 1,
  STATUS_BUSY = 2,
  STATUS_NO_CONNECTION = 3,
};
