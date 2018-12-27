#include "position.h"

void logPosition(std::string && str);

int Position::makeOrder(Instrument & instr, double totalBuy)
{
    const candle &last = instr._candles.back();
    std::cout << "New order: Price" << last.close << " size " << totalBuy << std::endl;
    _v1.newOrder(instr._v1name,
                    totalBuy * 1.0,
                    last.close,
                    "buy",
                    "exchange limit",
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

    instr.setOrder(_v1.strResponse(), last, totalBuy);
    logPosition(std::string(" - Buy order: ") + _v1.strResponse());

    return (0);
}

int Position::shortOrder(Instrument & instr)
{
    const candle &last = instr._candles.back();
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
    Document document;
    document.Parse(_v1.strResponse().c_str());
    instr.clearOrder();

    std::cout << _v1.strResponse() << std::endl;

    logPosition(std::string(" - Sell order: ") + _v1.strResponse());
    return (0);
}

int Position::isMacdReducing(Instrument &i, bool sell = false)
{
    const candle &last = i._candles.back();
    i._candles.pop_back();
    const candle &prelast = i._candles.back();

    if (sell == false && prelast.macdHistogram < last.macdHistogram)
    {
        std::cout << "Macd reducing - Past: " << prelast.macdHistogram << " Current: " << last.macdHistogram << std::endl;
        return (0);
    }
    else if (sell == true && prelast.macdHistogram > last.macdHistogram)
    {
        return (0);
    }
    return -1;
}

int Position::makePosition(Instrument &i, Wallet & _wallet)
{
    const candle &last = i._candles.back();
    double unitPrice = last.close * _wallet.lastPrice;

    double totalBuy = 18 / unitPrice;

    std::cout << "Value to buy: " << totalBuy << std::endl;
    std::cout << "Price to buy: " << totalBuy * unitPrice << std::endl;

    if (last.rsi < 32)
    {
        if (isMacdReducing(i, false) == 0)
        {
            std::cout << "Buying position" << std::endl;
            makeOrder(i, totalBuy);
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

int Position::shortPosition(Instrument &i)
{
    const candle &last = i._candles.back();
    std::cout << " trying to sell: " << i._v1name << std::endl;
    std::cout << " OrderID: " << i.orderId << std::endl;
    std::cout << " OrderPrice: " << i.orderPrice << std::endl;
    std::cout << " OrderSize: " << i.orderSize << std::endl;
    std::cout << " OrderSize: " << i.orderSize << std::endl;
    std::cout << " Orig Amount: " << i.originalAmount << std::endl;
    std::cout << " Orig Available: " << i.executedAmount << std::endl;
    if (last.rsi > 65)
    {
        std::cout << "RSI high" << std::endl;
        if (isMacdReducing(i, true))
        {
            shortOrder(i);
            return (0);
        }
        else
        {
            std::cout << "MACD: Price still increasing" << std::endl;
        }
    }
    else if (i.orderPrice * 0.93 > last.close) {
        std::cout << "Price decreasing too much: shorting position" << std::endl;
        shortOrder(i);
        return (0);
    }
    else
    {
        std::cout << "rsi too low to sell" << std::endl;
    }
    return (-1);
}