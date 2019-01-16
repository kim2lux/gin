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
                 tosell * 0.998,
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

int Position::isPositionOk(Instrument &instr, double totalBuy, const std::time_t &ts)
{
    candle &last = instr._candles.back();
    logPosition(std::string("want to buy Qty: " + to_string(totalBuy) + " Price: " + to_string(last.close)));
    _v1.getOrderBook(instr._v1name);
    logPosition(std::string("Response: " + _v1.strResponse()));

    if (_v1.hasApiError() != 0)
        return (-1);

    Document document;
    document.Parse(_v1.strResponse().c_str());
    Value &data = document["asks"];
    std::cout << "debug 1" << std::endl;
    Value &ask = data[0];
    std::cout << "debug 2" << std::endl;
    double posPrice = atof(ask["price"].GetString());
    double posAmount = atof(ask["amount"].GetString());
    double posTimestamp  = atof(ask["timestamp"].GetString());

    std::cout << "Position price: " << posPrice << " Want: " << last.close
              << " Position Amount: " << posAmount << " Needed" << totalBuy
              << " Position Timestamp: " << posTimestamp << " Current " << ts << std::endl;

    if (posPrice <= last.close * 1.005 && posAmount > totalBuy && posTimestamp + 60 > ts)
        return (0);
    return (-1);
}

int Position::makeOrder(Instrument &instr, double totalBuy)
{
    double buyTest = 0;
    buyTest = _wallet.getMinimumOrderSize(instr._v1name, _v1);
    if (buyTest != 0)
        totalBuy = buyTest;
    totalBuy *= 1.02;
    const candle &last = instr._candles.back();
    const std::time_t ts = last.timestamp / 1000;
    std::cout << std::ctime(&ts) << std::endl;
    int iterbuy = 0;
    while (isPositionOk(instr, totalBuy, ts) != 0 && iterbuy != 10)
    {
        logPosition(std::string(std::string(ctime(&ts)) + " - Position NOK: " + _v1.strResponse() + " " + instr._v1name));
        sleep(2);
        iterbuy++;
    }
    if (iterbuy == 10)
        return (-1);
    exit(-1); // just to check :)
    logPosition(std::string(std::string(ctime(&ts)) + " - Position is GOOD: " + _v1.strResponse() + " " + instr._v1name));
    _v1.newOrder(instr._v1name,
                 totalBuy,
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
        {
            instr.updateWallet();
            makeSellOrder(instr, totalBuy);
            logPosition("SELL what we can :(");
            return (0);
        }
        logPosition(std::string(std::string(ctime(&ts)) + " - Buy order OK: " + _v1.strResponse() + " " + instr._v1name + " Current Price: " + std::to_string(last.close)));
        sleep(1);
        instr.updateWallet();

        if (makeSellOrder(instr, totalBuy) == -1)
        {
            logPosition("Buy OK, Error SELL");
            exit(0);
        }
    }
    return (0);
}

int Position::shortOrder(Instrument &instr)
{
    const candle &last = instr._candles.back();

    std::cout << "selling: orderId : " << instr.orderBuyId << " buy price : " << instr.orderBuyPrice << " Amount size: " << instr.executedBuyAmount << " at Price: " << last.close;
    _v1.newOrder(instr._v1name,
                 instr.executedBuyAmount * 0.998,
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
        {
            logPosition(" SELL LOOZ: " + _v1.strResponse());
            return (-1);
        }
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

bool isHma(Instrument &i)
{
    auto end = i._candles.end();
    std::advance(end, -3);
    double tmp = end->hma;
    while (end != i._candles.end())
    {
        if (end->hma < tmp)
            return false;
        tmp = end->hma;
        end++;
    }
    return true;
}

int Position::makePosition(Instrument &i, Wallet &_wallet, bool simu = false, bool backtest = false)
{
    const candle last = i._candles.back();
    auto prelast = i._candles.end();
    std::advance(prelast, -2);

    //i._candles.pop_back();
    //const candle &prelast = i._candles.back();

    double totalBuy = 15 / last.close;
    //if (last.adx > 26 && last.plus_di > last.minus_di && last.adx >  prelast->adx)
    //if (last.rsi < 24)
    bool hmaPosition = isHma(i);
    if (hmaPosition == true && prelast->rsi < 25)
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
            i.orderSellId = 0xff;
            i.orderBuyPrice = last.close;
            i.averageBuyPrice = last.close;

            i.position = true;
            i.orderBuySize = totalBuy * 0.999;
            i.executedBuyAmount = totalBuy * 0.999;
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

    if (backtest == true && i.averageBuyPrice * 1.004 < end->close)
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
            i.orderSellSize = i.orderBuySize * 0.998;
            logSell("Winning sell: ", i, last, *end);
            i.clearOrder();
        }
    }
    else if (i.averageBuyPrice * 0.98 > end->close && end->hma > last.hma)
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