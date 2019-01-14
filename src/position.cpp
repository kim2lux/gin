#include "position.h"
#include <ctime>
void logPosition(std::string &&str);
#include <iomanip>
#include <sstream>

double totalDiff = 0;

void logSell(std::string &&res, Instrument &i, const candle &last, const candle &prelast)
{
    std::stringstream stream;
    std::time_t ts = last.timestamp / 1000;
    double diff = (i.averageSellPrice - i.averageBuyPrice) * i.orderBuySize;

    totalDiff += diff;

    stream << res << std::string(ctime(&ts))
           << std::fixed << std::setprecision(9) << " Diff Sell - Buy: " << diff
           << " Money: " << diff
           << " Total: " << totalDiff
           << " Name: " << i._v1name
           << " Sell Order Price: " << std::to_string(i.averageSellPrice)
           << " Quantity: " << std::to_string(i.orderSellSize);
    string sdiff = stream.str();
    logPosition(std::string(sdiff));
}

void logBuy(std::string &&res, Instrument &i, const candle &last)
{
    std::stringstream stream;
    std::time_t ts = last.timestamp / 1000;
    stream << res << std::string(ctime(&ts)) << " instrument: " << i._v1name
           << std::fixed << std::setprecision(9)
           << " Current Price: " << std::to_string(last.close)
           << " OrderSize: " << i.orderBuySize
           << " OrderPrice: " << i.orderBuyPrice
           << " Total dollar" << i.orderBuySize * i.orderBuyPrice;

    string sdiff = stream.str();
    logPosition(std::string(sdiff));
}

int Position::makeSellOrder(Instrument &instr, double totalBuy)
{
    const candle &last = instr._candles.back();
    const std::time_t ts = last.timestamp / 1000;
    std::cout << std::ctime(&ts) << std::endl;
    double tosell = instr._available;
    if (instr._available < instr.executedBuyAmount)
        tosell = instr.executedBuyAmount;
    _v1.newOrder(instr._v1name,
                 //instr.executedBuyAmount,
                 tosell,
                 //instr.averageBuyPrice * 1.03,
                 instr.averageBuyPrice * 1.004,
                 "sell",
                 "exchange limit",
                 false,
                 true,
                 false,
                 false,
                 0);
    if (_v1.hasApiError() != 0)
    {
        logPosition(std::string(std::string(ctime(&ts)) + " - Sell order NOK: " + _v1.strResponse() + " " + instr._v1name));
        return (-1);
    }
    else
    {
        if (instr.setSellOrder(_v1.strResponse(), last, totalBuy) == -1)
            return (-1);
        logPosition(std::string(std::string(ctime(&ts)) + " - Sell order OK: " + _v1.strResponse() + " " + instr._v1name + " Sell Size: " + std::to_string(instr.executedBuyAmount) + " Sell Price: " + std::to_string(instr.averageBuyPrice * 1.03)));
    }
    return (0);
}

int Position::makeOrder(Instrument &instr, double totalBuy)
{
    const candle &last = instr._candles.back();
    const std::time_t ts = last.timestamp / 1000;
    std::cout << std::ctime(&ts) << std::endl;
    _v1.newOrder(instr._v1name,
                 totalBuy * 1.02,
                 last.close,
                 "buy",
                 "exchange market",
                 false,
                 true,
                 false,
                 false,
                 0);
    if (_v1.hasApiError() != 0)
    {
        logPosition(std::string(std::string(ctime(&ts)) + " - Buy order NOK: " + _v1.strResponse() + " " + instr._v1name));
        return (-1);
    }
    else
    {
        if (instr.setBuyOrder(_v1.strResponse(), last, totalBuy) == -1)
            return (-1);
        logPosition(std::string(std::string(ctime(&ts)) + " - Buy order OK: " + _v1.strResponse() + " " + instr._v1name + " Current Price: " + std::to_string(last.close)));
        sleep(1);
        instr.updateWallet();
        double totalBuy = 0;
        totalBuy = _wallet.getMinimumOrderSize(instr._v1name, _v1);
        if (makeSellOrder(instr, totalBuy) == -1)
        {
            logPosition("Buy OK, Error SELL");
        }
    }
    return (0);
}

int Position::shortOrder(Instrument &instr)
{
    const candle &last = instr._candles.back();

    std::cout << "selling: orderId : " << instr.orderBuyId << " buy price : " << instr.orderBuyPrice << " Amount size: " << instr.executedBuyAmount << " at Price: " << last.close;
    _v1.newOrder(instr._v1name,
                 instr.executedBuyAmount,
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
        if (instr.setSellOrder(_v1.strResponse(), last, 0) == -1)
            return (-1);
        return (0);
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

void logFalse(Instrument &i, const candle &last, double totalBuy)
{

    std::stringstream stream;
    std::time_t ts = last.timestamp / 1000;
    stream << std::string(ctime(&ts)) << " instrument: " << i._v1name
           << std::fixed << std::setprecision(9)
           << " Fail Price: " << std::to_string(last.close)
           << " Fail Size: " << std::to_string(totalBuy)
           << " Fail total price: " << std::to_string(last.close * totalBuy);

    string sdiff = stream.str();
    logPosition(std::string(sdiff));
}

int Position::makePosition(Instrument &i, Wallet &_wallet, bool simu = false, bool backtest = false)
{
    const candle last = i._candles.back();
    auto prelast = i._candles.end();
    std::advance(prelast, -2);

    //i._candles.pop_back();
    //const candle &prelast = i._candles.back();

    double totalBuy = 15 / last.close;
    //if (last.adx > 21 && last.plus_di > last.minus_di && last.adx >  prelast->adx)
    if (last.rsi < 24)
    {

        std::cout << "BUYING POSITION: size: "
                  << totalBuy << " price: "
                  << last.close << std::endl;

        if (simu == false && backtest == false)
        {
            if (makeOrder(i, totalBuy) != 0)
                logFalse(i, last, totalBuy);
        }
        else
        {
            i.orderBuyId = 0xff;
            i.orderBuyPrice = last.close;
            i.averageBuyPrice = last.close;

            i.position = true;
            i.orderBuySize = totalBuy;
            i.executedBuyAmount = totalBuy;
        }
        logBuy("Buy: ", i, last);
    }
    return (0);
}

int Position::shortPosition(Instrument &i, Wallet &wallet, bool simu = false, bool backtest = false)
{
    const candle last = i._candles.back();
    auto end = i._candles.end();
    std::advance(end, -2);

    if (backtest == true && i.averageBuyPrice * 1.03 < last.close) // && last.hma <= end->hma)
    {

        if (simu == false && backtest == false)
        {
            if (shortOrder(i) == 0)
            {
                logSell("Real ** Winning sell: ", i, last, *end);
            }
        }
        else
        {
            i.averageSellPrice = last.close;
            i.orderSellSize = i.orderBuySize;
            logSell("Winning sell: ", i, last, *end);
            i.clearOrder();
        }
    }
    else if (i.averageBuyPrice * 0.97 > last.close)
    {
        if (simu == false && backtest == false)
        {
            _v1.cancelOrder(i.orderSellId);
            logPosition("Cancel Limit sell for market sell => ID:" + to_string(i.orderSellId));
            sleep(1);
            shortOrder(i);
            logSell("Real ** Loosing sell: ", i, last, *end);
        }
        else
        {
            i.averageSellPrice = last.close;
            i.orderSellSize = i.orderBuySize;
            logSell("Loosing sell: ", i, last, *end);
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