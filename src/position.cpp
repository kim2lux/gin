#include "position.h"
#include <ctime>
void logPosition(std::string &&str);
#include <iomanip>
#include <sstream>

double totalDiff = 0;

void logSell(std::string &&res, Instrument &i, Wallet &wallet, const candle &last, const candle &prelast)
{
    std::stringstream stream;
    std::time_t ts = last.timestamp / 1000;
    double diff = (last.close - i.orderPrice) * i.orderSize;

    totalDiff += diff;

    stream << res << std::string(ctime(&ts))
           << std::fixed << std::setprecision(9) << " Diff: " << diff
           << " Money: " << diff * wallet.lastPrice
           << " Total: " << totalDiff * wallet.lastPrice
           << " Name: " << i._v1name
           << " Current Price: " << std::to_string(last.close)
           << " Current HMA: " << std::to_string(last.hma)
           << " Past    HMA: " << std::to_string(prelast.hma)
           << " Buy Order Price: " << std::to_string(i.orderPrice)
           << " Quantity: " << std::to_string(i.orderSize);
    string sdiff = stream.str();
    logPosition(std::string(sdiff));
}

void logBuy(std::string &&res, Instrument &i, Wallet &wallet, const candle &last)
{
    std::stringstream stream;
    std::time_t ts = last.timestamp / 1000;
    stream << res << std::string(ctime(&ts)) << " instrument: " << i._v1name
           << std::fixed << std::setprecision(9)
           << " Current Price: " << std::to_string(last.close)
           << " OrderSize: " << i.orderSize
           << " OrderPrice: " << i.orderPrice
           << " TotalOrder: " << i.orderSize * i.orderPrice * wallet.lastPrice
           << " Total Btc" << i.orderSize * i.orderPrice;

    string sdiff = stream.str();
    logPosition(std::string(sdiff));
}

int Position::makeOrder(Instrument &instr, double totalBuy)
{
    const candle &last = instr._candles.back();
    const std::time_t ts = last.timestamp / 1000;
    std::cout << std::ctime(&ts) << std::endl;
    _v1.newOrder(instr._v1name,
                 totalBuy * 1.5,
                 last.close / 3,
                 "buy",
                 "exchange limit",
                 false,
                 true,
                 false,
                 false,
                 0);
    if (_v1.hasApiError() != 0)
    {
        logPosition(std::string(std::string(ctime(&ts)) + " - Buy order NOK: " + _v1.strResponse() + " " + instr._v1name + " Current Price: " + std::to_string(last.close)));
    }
    else
    {
        instr.setOrder(_v1.strResponse(), last, totalBuy);
        logPosition(std::string(std::string(ctime(&ts)) + " - Buy order OK: " + _v1.strResponse() + " " + instr._v1name + " Current Price: " + std::to_string(last.close)));
    }
    return (0);
}

int Position::shortOrder(Instrument &instr)
{
    const candle &last = instr._candles.back();
    double diff = last.close - instr.orderPrice;

    std::time_t ts = last.timestamp / 1000;

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
    else if (sell == true && prelast.macdHistogram > last.macdHistogram)
    {
        std::cout << "Macd reducing - Past: " << prelast.macdHistogram << " Current: " << last.macdHistogram << std::endl;
        return (0);
    }
    return -1;
}

int Position::makePosition(Instrument &i, Wallet &_wallet, bool simu = false, bool backtest = false)
{
    const candle last = i._candles.back();
    auto prelast = i._candles.end();
    std::advance(prelast, -2);

    //i._candles.pop_back();
    //const candle &prelast = i._candles.back();

    //if (last.adx > 21 && last.plus_di > last.minus_di && last.adx >  prelast->adx)
    if (last.rsi < 25)
    {
        double unitPrice = last.close * _wallet.lastPrice;
        double totalBuy = 3 / unitPrice;

        if (simu == false && backtest == false)
        {
            makeOrder(i, totalBuy);
        }
        else
        {
            i.orderId = 0xff;
            i.orderPrice = last.close;
            i.position = true;
            i.orderSize = totalBuy;
        }
        logBuy("Buy: ", i, _wallet, last);
    }
    return (0);
}

int Position::shortPosition(Instrument &i, Wallet &wallet, bool simu = false, bool backtest = false)
{
    const candle last = i._candles.back();
    auto end = i._candles.end();
    std::advance(end, -2);

    if (i.orderPrice * 1.03 < last.close)// && last.hma <= end->hma)
    {
        logSell("Winning sell: ", i, wallet, last, *end);
        if (simu == false && backtest == false)
        {
            shortOrder(i);
        }
        else
        {
            i.clearOrder();
        }
    }
    else if (i.orderPrice * 0.97 > last.close)
    {
        logSell("Loosing sell: ", i, wallet, last, *end);
        if (simu == false && backtest == false)
        {
            shortOrder(i);
        }
        else
        {
            i.clearOrder();
        }
        return (0);
    }
    else
    {
        std::cout << "Not selling yet !" << std::endl;
    }
    return (-1);
}