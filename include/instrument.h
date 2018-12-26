#pragma once

#include "candle.h"
#include <list>
#include "bfxApiV2.hpp"
#include "BitfinexAPI.hpp"
#include <unistd.h>

using namespace BfxAPI;

class instrument {
public:
    instrument(std::string v1name, std::string v2name, BfxAPI::bitfinexAPIv2 & bfx, BfxAPI::BitfinexAPI & v1, candleInterface &iCandle) : 
    _v1name(v1name), _v2name(v2name), _bfx(bfx), _apiv1(v1), _icandle(iCandle)  {
        orderId = 0;
        orderPrice = 0;
        orderSize = 0;
        originalAmount = 0;
        executedAmount = 0;
    }

    std::list<candle>   _candles;
    std::string         _v1name;
    std::string         _v2name;
    //void updateCandles();
    const BfxAPI::bitfinexAPIv2 & _bfx;
    BfxAPI::BitfinexAPI & _apiv1;
    candleInterface & _icandle;
    void updateRsi();
    void updateCandles(bool replay = false, const char * filepath = nullptr);
    void updateMacd();
    void display();
    void makeOrder(double);
    void shortOrder();

    bool position = false;
    int64_t orderId;
    double orderPrice;
    double orderSize;
    double originalAmount;
    double executedAmount;

private:

};