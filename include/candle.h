#pragma once

#include <vector>
#include <stdint.h>
#include <iostream>

#include "bfxApiV2.hpp"
using namespace BfxAPI;

class instrument;

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

namespace BfxAPI
{

class candleInterface
{
public:
  candleInterface(std::string time, bitfinexAPIv2 &bfxApi) : _time(time), _bfxApi(bfxApi)
  {
  }
  std::list<candle> retrieveCandles(instrument &instr, const char *filepath = nullptr);
  std::list<candle> pushCandles(std::string json);
  bool getLastCandle(instrument &instr);

  const std::string &_time;
  bitfinexAPIv2 &_bfxApi;
};

} // namespace BfxAPI