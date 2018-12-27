#pragma once

#include "BitfinexAPI.hpp"

class Wallet
{
public:
  double available;
  double amount;
  double lastPrice;

public:
  int update(BfxAPI::BitfinexAPI &v1);
};
