#pragma once

#include <vector>
#include <stdint.h>
#include <iostream>

#include <list>

#include "bfxApiV2.hpp"
using namespace BfxAPI;

class Instrument;

struct candle
{
  candle(uint64_t ts, double o, double c,
         double h, double l, double v) : timestamp(ts), open(o), close(c), high(h), low(l), volume(v)
  {
    rsi = 0.0;
    macdHistogram = 0.0;
    macdSignal = 0.0;
    macdHistogram = 0.0;
  }
  uint64_t timestamp;
  double open;
  double close;
  double high;
  double low;
  double volume;
  double rsi;
  double macdHistogram;
  double macdSignal;
  double macd;
};

class CandleInterface
{
public:
  CandleInterface(bitfinexAPIv2 &bfxApi);
  std::list<candle> retrieveCandles(Instrument &instr, const char *filepath = nullptr);
  std::list<candle> pushCandles(std::string json);
  void save(std::string json, std::string &name);
  bool getLastCandle(Instrument &instr);

  bitfinexAPIv2 &_bfxApi;
};
