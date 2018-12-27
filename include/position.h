#pragma once
#include "instrument.h"
#include "wallet.h"
#include <iostream>

class Position
{
  public:

    BfxAPI::BitfinexAPI & _v1;

    Position(BfxAPI::BitfinexAPI & v1) : _v1(v1) {
      
    }

    int shortOrder(Instrument & instr);
    int makeOrder(Instrument & instr, double totalBuy);
    int makePosition(Instrument &i,  Wallet &);
    int shortPosition(Instrument &i);
    int isMacdReducing(Instrument &i, bool sell);
};
