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
    int makePosition(Instrument &i,  Wallet &, bool);
    int shortPosition(Instrument &i, bool);
    int isMacdReducing(Instrument &i, bool sell);
};
