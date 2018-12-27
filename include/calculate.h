#pragma once

#include "config.h"
#include "instrument.h"

class Calculate
{
  public:
    void updateMacd(Instrument &instr);
    void updateRsi(Instrument &instr);
};
