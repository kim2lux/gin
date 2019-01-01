#pragma once

#include "config.h"
#include "instrument.h"

class Calculate
{
public:
  void updateHma(Instrument &instr);
  void updateMacd(Instrument &instr);
  void updateRsi(Instrument &instr);
};
