#include "position.h"
#include <ctime>
void logPosition(std::string &&str);

int Position::makeOrder(Instrument &instr, double totalBuy)
{
    const candle &last = instr._candles.back();
    const std::time_t ts = last.timestamp;
    std::cout << std::ctime(&ts) << std::endl;
    logPosition(std::string(std::string(ctime(&ts)) + " - Buy order: " + instr._v1name + " Current Price: " + std::to_string(last.close)));
    _v1.newOrder(instr._v1name,
                 totalBuy,
                 last.close / 2,
                 "buy",
                 "exchange limit",
                 false,
                 true,
                 false,
                 false,
                 0);
    if (_v1.hasApiError() != 0)
    {
        instr.orderId = 0xff; //only debug
        instr.orderPrice = last.close;
    }
    else
    {
        instr.setOrder(_v1.strResponse(), last, totalBuy);
    }
    return (0);
}

int Position::shortOrder(Instrument &instr)
{
    const candle &last = instr._candles.back();
    double diff = last.close - instr.orderPrice;

    std::time_t ts = last.timestamp;

    logPosition(std::string(std::string(ctime(&ts)) + " - Sell order: " + instr._v1name + " Price: " + std::to_string(last.close) + " Diff: " + std::to_string(diff)));
    std::cout << "selling: orderId : " << instr.orderId << " buy price : " << instr.orderPrice << " Amount size: " << instr.executedAmount << " at Price: " << last.close;
    _v1.newOrder(instr._v1name,
                 instr.executedAmount,
                 last.close,
                 "sell",
                 "exchange market",
                 false,
                 true,
                 false,
                 false,
                 0);
    if (_v1.hasApiError() != 0)
    {
        std::cout << "Error making the order" << std::endl;
        std::cout << _v1.strResponse() << std::endl;
        return (-1);
    }
    else
    {
        Document document;
        document.Parse(_v1.strResponse().c_str());
        instr.clearOrder();
        std::cout << _v1.strResponse() << std::endl;
    }
    return (0);
}

int Position::isMacdReducing(const candle &last, const candle &prelast, bool sell = false)
{
    if (sell == false && prelast.macdHistogram < last.macdHistogram)
    {
        std::cout << "Macd reducing - Past: " << prelast.macdHistogram << " Current: " << last.macdHistogram << std::endl;
        return (0);
    }
    else if (sell == true &&  prelast.macdHistogram > last.macdHistogram)
    {
        std::cout << "Macd reducing - Past: " << prelast.macdHistogram << " Current: " << last.macdHistogram << std::endl;
        return (0);
    }
    return -1;
}

int Position::makePosition(Instrument &i, Wallet &_wallet, bool simu = false)
{

    const candle &last = i._candles.back();
    i._candles.pop_back();
    const candle &prelast = i._candles.back();
    double unitPrice = last.close * _wallet.lastPrice;
    double totalBuy = 20 / unitPrice;

    if (last.rsi < 35)
    {
        if (isMacdReducing(last, prelast, false) == 0)
        {
            std::cout << "Buying position" << std::endl;
            if (simu == false)
            {
                std::cout << "Value to buy: " << totalBuy << std::endl;
                std::cout << "Price to buy: " << totalBuy * unitPrice << std::endl;
                makeOrder(i, totalBuy);
            }
            else
            {
                logPosition(std::string(" - Buy order: " + i._v1name + " Current Price: " + std::to_string(last.close)));
                i.orderId = 0xff;
                i.orderPrice = last.close;
            }
        }
        else
        {
            std::cout << "MACD decreasing" << std::endl;
        }
    }
    else
    {
        std::cout << "RSI too high" << std::endl;
    }
    return (0);
}

int Position::shortPosition(Instrument &i, bool simu = false)
{
    const candle &last = i._candles.back();
    i._candles.pop_back();
    const candle &prelast = i._candles.back();

    std::cout << " trying to sell: " << i._v1name << std::endl;
    std::cout << " OrderID: " << i.orderId << std::endl;
    std::cout << " OrderPrice: " << i.orderPrice << std::endl;
    std::cout << " OrderSize: " << i.orderSize << std::endl << std::endl;

    std::cout << " Diff HMA: " << last.hma - prelast.hma << std::endl;
    std::cout << " Diff MACD: " << last.macdHistogram - prelast.macdHistogram << std::endl;
    if (last.rsi > 50)
    {
        std::cout << "RSI high" << std::endl;
        if (isMacdReducing(last, prelast, true) == 0 && prelast.hma >= last.hma)
        {
            logPosition(std::string(" - Sell : " + i._v1name + " Current Price: " + std::to_string(last.close) + " Order Price: " + std::to_string(i.orderPrice)));
            std::cout << "DEBUG" << std::endl;

            if (simu == false)
            {
                shortOrder(i);
            }
            else
            {
                i.orderId = 0;
                i.orderPrice = 0;
                i.position = 0;
            }
            return (0);
        }
        else
        {
            std::cout << "MACD: Price still increasing" << std::endl;
        }
    }
    else if (i.orderPrice * 0.93 > last.close)
    {
        logPosition(std::string(" - Sell Price decreasing too much 7%: shorting position : " + i._v1name + " Current Price: " + std::to_string(last.close) + " Order Price: " + std::to_string(i.orderPrice)));
        std::cout << "Price decreasing too much 7%: shorting position" << std::endl;
        shortOrder(i);
        return (0);
    }
    else
    {
        std::cout << "Not selling yet !" << std::endl;
    }
    return (-1);
}