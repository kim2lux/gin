#include "instrument.h"
#include "indicators.h"
#include "config.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <time.h>
void logPosition(std::string &&str);

double totalRealDiff = 0;

void Instrument::clearBuyOrder()
{
    orderBuyId = 0;

    orderBuyPrice = 0;
    orderSellSize = 0;

    originalBuyAmount = 0;
    executedBuyAmount = 0;

    averageBuyPrice = 0;

    buyTimestamp = 0;
    orderBuyPrice = 0;
    orderBuySize = 0;
}

void Instrument::clearSellOrder()
{
    orderSellId = 0;

    orderSellPrice = 0;
    orderSellSize = 0;

    originalSellAmount = 0;
    executedSellAmount = 0;

    averageSellPrice = 0;

    sellTimestamp = 0;
    orderSellPrice = 0;
    orderSellSize = 0;
}

void Instrument::initOrder(std::string json)
{
    std::cout << json << std::endl;
    Document document;
    document.Parse(json.c_str());
    Value &data = document;
    if (data.IsArray() == false)
    {
        std::cout << "no active orders" << std::endl;
        clearBuyOrder();
        clearSellOrder();
        return;
    }
    for (auto &it : data.GetArray())
    {
        if (strcmp(it["symbol"].GetString(), _v1name.c_str()) == 0)
        {
            if (strcmp(it["side"].GetString(), "buy") == 0)
            {
                orderBuyId = it["id"].GetDouble();
                orderBuyPrice = atof(it["price"].GetString());
                executedBuyAmount = atof(it["executed_amount"].GetString());
                originalBuyAmount = atof(it["original_amount"].GetString());
                averageBuyPrice = atof(it["avg_execution_price"].GetString());
            }
            else if (strcmp(it["side"].GetString(), "sell") == 0)
            {
                orderSellId = it["id"].GetDouble();
                orderSellPrice = atof(it["price"].GetString());
                executedSellAmount = atof(it["executed_amount"].GetString());
                originalSellAmount = atof(it["original_amount"].GetString());
            }
        }
    }
}

Instrument::Instrument(std::string v1name, std::string v2name,
                       BfxAPI::bitfinexAPIv2 &bfx, BfxAPI::BitfinexAPI &v1,
                       CandleInterface &iCandle,
                       const std::string jsonOrders) : _v1name(v1name), _v2name(v2name), _bfx(bfx), _apiv1(v1), _icandle(iCandle)
{
    position = false;
    orderBuyId = 0;
    orderSellId = 0;
    orderBuyPrice = 0;
    orderBuySize = 0;
    originalBuyAmount = 0;
    executedBuyAmount = 0;

    orderSellPrice = 0;
    orderSellSize = 0;
    originalSellAmount = 0;
    executedSellAmount = 0;
    backTest = false;
    _candlePosition = 0;
    //if (!jsonOrders.empty())
    //    initOrder(jsonOrders);
}

void Instrument::updateFromAll()
{
    _candles.pop_front();
    auto iter = _totalCandles.begin();
    std::advance(iter, _candlePosition);
    _candles.push_back(*iter);
    _candlePosition++;
}

void Instrument::loadCandleFromAll()
{
    _candles.clear();
    int i = 0;
    auto iter = _totalCandles.begin();
    //    std::advance(iter, _candlePosition);
    while (i < 100)
    {
        _candles.push_back(*iter);
        iter++;
        i++;
        _candlePosition++;
    }
}

void Instrument::updateCandles(bool replay, const char *filepath)
{
    _candles.clear();

    if (replay == true)
    {
        _candles = std::move(_icandle.retrieveCandles(*this, filepath, 0));
    }
    else
    {
        _candles = std::move(_icandle.retrieveCandles(*this, nullptr, 100));
    }
}

int Instrument::setSellOrder(std::string response, const candle &last, double totalBuy)
{
    Document document;
    document.Parse(response.c_str());
    Value &data = document;

    if (data.HasMember("message"))
    {
        logPosition({"Not able to sell: " + response});
        return (-1);
    }
    else
    {
        std::cout << "orderId" << data["id"].GetInt64() << std::endl;

        //        sellTimestamp = atof(data["original_amount"].GetString());
        orderSellId = data["id"].GetInt64();
        originalSellAmount = atof(data["original_amount"].GetString());
        executedSellAmount = atof(data["executed_amount"].GetString());
        averageSellPrice = atof(data["avg_execution_price"].GetString());

        orderSellPrice = last.close;
        orderSellSize = executedSellAmount;
        position = true;
    }
    return (0);
}

int Instrument::updateWallet()
{
    
    _apiv1.getBalances();
    if (_apiv1.hasApiError() != 0)
    {
        std::cerr << "error retrieving wallet" << std::endl;
        return (-1);
    }
    Document document;
    document.Parse(_apiv1.strResponse().c_str());
    Value &data = document;
    std::cout << _apiv1.strResponse() << std::endl;

    for (auto &it : data.GetArray())
    {
        assert(it.HasMember("currency"));
        std::cout << "currency check" << it["currency"].GetString() << std::endl;
        if (strncmp(it["currency"].GetString(), this->_v1name.c_str(), 3) == 0)
        {
            assert(it.HasMember("available"));
            _available = atof(it["available"].GetString());
            assert(it.HasMember("amount"));
            _amount = atof(it["amount"].GetString());
            logPosition("Available Amout to sell: " + to_string(_available));
        }
    }
    return (0);
}


int Instrument::setBuyOrder(std::string response, const candle &last, double totalBuy)
{
    Document document;
    document.Parse(response.c_str());
    Value &data = document;

    if (data.HasMember("message"))
    {
        logPosition({"Not able to buy: " + response});
        return (-1);
    }
    else
    {
        std::cout << "orderId" << data["id"].GetInt64() << std::endl;
        buyTimestamp = atof(data["original_amount"].GetString());
        orderBuyId = data["id"].GetInt64();
        orderBuyPrice = atof(data["price"].GetString());
        originalBuyAmount = atof(data["original_amount"].GetString());
        executedBuyAmount = atof(data["executed_amount"].GetString());
        averageBuyPrice = atof(data["avg_execution_price"].GetString());

        position = true;
    }
    int i = 0;
    while (originalBuyAmount > executedBuyAmount && i < 5)
    {
        sleep(1);
        std::cout << "Waiting order to be executed: " << std::endl;
        _apiv1.getOrderStatus(orderBuyId);

        if (_apiv1.hasApiError())
        {
            std::cout << "Cannot retrieve order status" << std::endl;
        }
        else
        {
            Document document;
            std::cout << _apiv1.strResponse() << std::endl;
            document.Parse(_apiv1.strResponse().c_str());
            Value &data = document;
            orderBuyPrice = atof(data["price"].GetString());
            originalBuyAmount = atof(data["original_amount"].GetString());
            executedBuyAmount = atof(data["executed_amount"].GetString());
            averageBuyPrice = atof(data["avg_execution_price"].GetString());
        }
        i++;
    }
    return (0);
}

void Instrument::clearOrder()
{
    position = false;
    orderBuyId = 0;
    orderSellId = 0;
    orderBuyPrice = 0;
    orderBuySize = 0;
    originalBuyAmount = 0;
    executedBuyAmount = 0;

    orderSellPrice = 0;
    orderSellSize = 0;
    originalSellAmount = 0;
    executedSellAmount = 0;
}

void Instrument::display()
{
    std::setprecision(9);
    candle &i = _candles.back();
    time_t a = i.timestamp / 1000;
    {
        std::cout
            << std::setprecision(12) << std::fixed << "Timestamp: " << ctime(&a)
            << " Instrument: " << _v1name << std::endl
            << " RSI: " << i.rsi << std::endl
            << " Close: " << i.close << std::endl
            << " ADX: " << i.adx << std::endl
            << " +DI: " << i.plus_di << std::endl
            << " -DI: " << i.minus_di << std::endl
            //<< " Macd: " << i.macd << std::endl
            //<< " Macd_signal: " << i.macdSignal << std::endl
            << " Macd_histo: " << i.macdHistogram << std::endl
            << " Hma: " << i.hma << std::endl
            << std::endl;
    }
}
