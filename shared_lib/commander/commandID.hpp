#pragma once
#include <Arduino.h>

enum CommandID
{
    CMD_HELP = 0,
    CMD_UPDATE = 1,
    CMD_SET = 2,
    CMD_MODE = 3,
    CMD_UPDATE_GAIN = 4,
    CMD_UPDATE_FREQMHZ = 5,
    CMD_UPDATE_SYNCWORD = 6,
    CMD_UPDATE_SF = 7,
    CMD_UPDATE_BW = 8,
    CMD_UPDATE_CR = 9,
    CMD_UPDATE_POWER = 10,
    CMD_CALIBRATE_CELL = 11,
    CMD_SET_OUTPUT = 12,

};