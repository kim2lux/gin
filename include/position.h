#pragma once
#include "instrument.h"
#include "wallet.h"
#include <iostream>

class Position
{
public:
  BfxAPI::BitfinexAPI &_v1;
  Wallet _wallet;

  Position(BfxAPI::BitfinexAPI &v1) : _v1(v1)
  {
  }

  int shortOrder(Instrument &instr);
  int makeOrder(Instrument &instr, double totalBuy);
  int makePosition(Instrument &i, Wallet &, bool, bool);
  int shortPosition(Instrument &i, Wallet &, bool, bool);
  int isMacdReducing(const candle &last, const candle &prelast, bool sell);
  int makeSellOrder(Instrument &instr, double totalBuy);
};
