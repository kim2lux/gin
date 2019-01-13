#pragma once

#include "instrument.h"
#include "config.h"
#include "wallet.h"
#include "calculate.h"
#include "position.h"
#include <iostream>
#include <vector>
using namespace BfxAPI;
const std::string currentDateTime();
void logPosition(std::string &&str);

class Api
{

public:
  Api(Config &config) : v2(config.accessKey, config.secretKey), v1(config.accessKey, config.secretKey)
  {
  }
  BfxAPI::bitfinexAPIv2 v2;
  BfxAPI::BitfinexAPI v1;
};

class Engine
{
public:
  Engine(Config &config);
  Config &_config;
  Api _api;
  Calculate _calc;
  Position _position;
  CandleInterface _candleInterface;
  std::vector<std::pair<std::string, std::string>> _instruments;
  std::vector<Instrument> vInstr;
  Wallet _wallet;
  std::vector<std::string> _simuCandles;

  int loadInstrument();
  int retrieveInstrument();
  void run();
  int updateInstrument(Instrument &instr);
  int initSimuCandles(Instrument &instr);
  int makeOrders(Instrument &instr);
  int isVolume(std::string v1name, int minVolume);
};
