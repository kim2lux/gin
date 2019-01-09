#pragma once

#include "candle.h"
#include <list>
#include "bfxApiV2.hpp"
#include "BitfinexAPI.hpp"
#include <unistd.h>
#include "order.h"

using namespace BfxAPI;
class bitfinexAPIv2;
class Instrument
{
public:
  Instrument(std::string v1name, std::string v2name,
             BfxAPI::bitfinexAPIv2 &bfx, BfxAPI::BitfinexAPI &v1,
             CandleInterface &iCandle, const std::string jsonOrders);

  std::list<candle> _candles;
  std::string _v1name;
  std::string _v2name;
  //void updateCandles();
  const BfxAPI::bitfinexAPIv2 &_bfx;
  BfxAPI::BitfinexAPI &_apiv1;
  CandleInterface &_icandle;

  void updateCandles(bool replay = false, const char *filepath = nullptr);
  void clearOrder();
  void setOrder(std::string response, const candle &last, double totalBuy);
  void display();


  bool position = false;
  int64_t orderId;
  double orderPrice;
  double orderSize;
  double originalAmount;
  double executedAmount;
  void initOrder(std::string json);
};
