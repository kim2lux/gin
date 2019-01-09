#include "position.h"
#include <ctime>
void logPosition(std::string &&str);
#include <iomanip>
#include <sstream>

double totalDiff = 0;

int Position::makeOrder(Instrument &instr, double totalBuy)
{
    const candle &last = instr._candles.back();
    const std::time_t ts = last.timestamp / 1000;
    std::cout << std::ctime(&ts) << std::endl;
    _v1.newOrder(instr._v1name,
                 totalBuy * 1.5,
                 last.close / 4,
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

int Position::makePosition(Instrument &i, Wallet &_wallet, bool simu = false)
{

    const candle last = i._candles.back();
    i._candles.pop_back();
    const candle &prelast = i._candles.back();
    double unitPrice = last.close * _wallet.lastPrice;
    double totalBuy = 20 / unitPrice;
    std::time_t ts = last.timestamp / 1000;
    if (last.rsi < 35)
    {
        if (isMacdReducing(last, prelast, false) == 0 && last.hma > prelast.hma)
        {
            logPosition(std::string(std::string(ctime(&ts)) + " - Buy order: " + i._v1name + " Current Price: " + std::to_string(last.close)));
            std::cout << "Buying position" << std::endl;
            if (simu == false)
            {
                makeOrder(i, totalBuy);
            }
            else
            {
                i.orderId = 0xff;
                i.orderPrice = last.close;
                i.position = true;
                i.orderSize = totalBuy;
                std::cout << i.orderPrice << std::endl;
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

int Position::shortPosition(Instrument &i, Wallet &wallet, bool simu = false)
{
    const candle last = i._candles.back();
    i._candles.pop_back(); // !! BUG HERE (using last in shortOrder func)
    const candle &prelast = i._candles.back();

    std::cout << " trying to sell: " << i._v1name << std::endl;
    std::cout << " OrderID: " << i.orderId << std::endl;
    std::cout << " OrderPrice: " << i.orderPrice << std::endl;
    std::cout << " OrderSize: " << i.orderSize << std::endl;

    std::cout << " Diff HMA: " << last.hma - prelast.hma << std::endl;
    std::cout << " Diff MACD: " << last.macdHistogram - prelast.macdHistogram << std::endl;

    std::time_t ts = last.timestamp / 1000;

    if (last.rsi > 60)
    {
        std::cout << "RSI high" << std::endl;
        if (isMacdReducing(last, prelast, true) == 0 && (last.hma - prelast.hma) < 0)
        {
            double diff = (last.close - i.orderPrice) * i.orderSize;
            totalDiff += diff;
            std::stringstream stream;
            stream << "Sell: " << std::string(ctime(&ts))
                   << std::fixed << std::setprecision(9) << " Diff: " << diff
                   << " Money: " << diff * wallet.lastPrice
                   << " Total: " << totalDiff * wallet.lastPrice
                   << " Name: " << i._v1name
                   << " Current Price: " << std::to_string(last.close)
                   << " Buy Order Price: " << std::to_string(i.orderPrice)
                   << " Quantity: " << std::to_string(i.orderSize);
            string sdiff = stream.str();
            logPosition(std::string(sdiff));
            if (simu == false)
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
            std::cout << "Price still increasing" << std::endl;
        }
    }
    else if (i.orderPrice * 0.95 > last.close)
    {
        logPosition(std::string(" - Sell Price decreasing too much 7%: shorting position : " + i._v1name + " Current Price: " + std::to_string(last.close) + " Order Price: " + std::to_string(i.orderPrice)));
        std::cout << "Price decreasing too much 7%: shorting position" << std::endl;
        if (simu == false)
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