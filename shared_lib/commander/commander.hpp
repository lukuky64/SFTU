#pragma once

#include <Arduino.h>

#include <cstring>

#include "LoRaCom.hpp"
#include "SerialCom.hpp"
#include "LoraMsg.hpp"
#include "commandID.hpp"

#ifdef SFTU
#include "ADCprocessing.hpp"
#include "actuation.hpp"
#include "adcADS.hpp"
#endif

#define c_cmp(a, b) (strcmp(a, b) == 0)

class Commander
{
public:
#ifdef SFTU
  Commander(SerialCom *serialCom, LoRaCom *loraCom, Actuation *actuation, adcADS *adcADS);
#else
  Commander(SerialCom *serialCom, LoRaCom *loraCom);
#endif

private:
  char *m_command[128]; // Buffer to store the command

  uint16_t m_timeout = 20'000; // 20 second timeout for commands

  SerialCom *m_serialCom; // Pointer to SerialCom instance
  LoRaCom *m_loraCom;     // Pointer to LoRaCom instance

#ifdef SFTU
  Actuation *m_actuation;
  adcADS *m_adcADS;
#endif

  typedef void (Commander::*Handler)();

  struct HandlerMap
  {
    const char *name;
    Handler handler;
  };

  void handle_update_gain(float param);
  void handle_update_freqMhz(float param);
  void handle_update_spreadingFactor(float param);
  void handle_update_bandwidthKHz(float param);
  void handle_calibrateCell(float param);
  void handle_set_OUTPUT(float param);

  // ----- Command Handlers -----
  void handle_command_help(); // Command handler for "help"
  void handle_update();       // Command handler for "update" parameters
  void handle_set();          // Command handler for "set" parameters
  void handle_mode();         // Command handler for "mode" (eg: transmit, receive,
                              // transcieve, spectrum scan, etc")

  // ----- Update Handlers -----
  void handle_update_help();            // Command handler for "help"
  void handle_update_gain();            // Command handler for "update gain"
  void handle_update_freqMhz();         // Command handler for "update freqMhz"
  void handle_update_spreadingFactor(); // Command handler for "update
                                        // spreading factor"
  void handle_update_bandwidthKHz();    // Command handler for "update bandwidth"

  void handle_calibrateCell(); // Command handler for "calibrateCell"
  // void handle_tareCell();       // Command handler for "tareCell"

  void handle_set_help();
  void handle_set_OUTPUT();

  void handle_help(const HandlerMap *handler);

  static constexpr const HandlerMap command_handler[5] = {{"help", &Commander::handle_command_help}, {"update", &Commander::handle_update}, {"set", &Commander::handle_set}, {"mode", &Commander::handle_mode}, {nullptr, nullptr}};

  static constexpr const HandlerMap update_handler[7] = {{"help", &Commander::handle_update_help},
                                                         {"gain", &Commander::handle_update_gain},
                                                         {"freqMHz", &Commander::handle_update_freqMhz},
                                                         {"sf", &Commander::handle_update_spreadingFactor},
                                                         {"bwKHz", &Commander::handle_update_bandwidthKHz},

                                                         {"calibrateCell", &Commander::handle_calibrateCell},
                                                         // {"tareCell", &Commander::handle_tareCell},
                                                         {nullptr, nullptr}};

  static constexpr const HandlerMap set_handler[3] = {{"help", &Commander::handle_set_help}, {"output", &Commander::handle_set_OUTPUT}, {nullptr, nullptr}};

  void runMappedCommand(char *command, const HandlerMap *handler);

  static constexpr const char *TAG = "Commander";

public:
  bool runCommand(uint8_t commandID, float param);

  void checkCommand(const HandlerMap *handler = command_handler); // Check the command and run
                                                                  // the appropriate handler

  void setCommand(const char *buffer);

  char *readAndRemove();
};
