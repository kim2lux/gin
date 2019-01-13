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
  std::list<candle> _totalCandles;
  int _candlePosition;

  std::string _v1name;
  std::string _v2name;

  const BfxAPI::bitfinexAPIv2 &_bfx;
  BfxAPI::BitfinexAPI &_apiv1;

  CandleInterface &_icandle;

  //wallet

  double _available;
  double _amount;
  int updateWallet();

  //order

  void initOrder(std::string json);
  void loadCandleFromAll();
  void updateFromAll();
  void updateCandles(bool replay = false, const char *filepath = nullptr);
  void clearOrder();

  int setBuyOrder(std::string response, const candle &last, double totalBuy);
  int setSellOrder(std::string response, const candle &last, double totalBuy);

  void clearBuyOrder();
  void clearSellOrder();

  void display();

  bool position = false;
  bool backTest = false;

  int64_t orderBuyId;
  int64_t orderSellId;

  double orderBuyPrice;
  double orderBuySize;

  double orderSellPrice;
  double orderSellSize;

  double originalBuyAmount;
  double executedBuyAmount;

  double originalSellAmount;
  double executedSellAmount;

  double averageBuyPrice;
  double averageSellPrice;

  double buyTimestamp;
  double sellTimestamp;
};
