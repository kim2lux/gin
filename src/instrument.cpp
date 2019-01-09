#include "instrument.h"
#include "indicators.h"
#include "config.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <time.h>
void logPosition(std::string &&str);

void Instrument::initOrder(std::string json)
{
    Document document;
    document.Parse(json.c_str());
    Value &data = document;
    if (data.IsArray() == false)
    {
        std::cout << "no active orders" << std::endl;
        return;
    }
    bool check = false;
    for (auto &it : data.GetArray())
    {
        if (strcmp(it["symbol"].GetString(), _v1name.c_str()) == 0)
        {
            check = true;
            std::cout << "Found symbol order: " << it["symbol"].GetString() << std::endl;
            orderId = it["id"].GetDouble();
            orderPrice = atof(it["price"].GetString());
            executedAmount = atof(it["executed_amount"].GetString());
            originalAmount = atof(it["original_amount"].GetString());
            std::cout << "orderId : " << orderId
                      << " original amount: " << originalAmount
                      << " executed amount:" << executedAmount
                      << " price buy: " << orderPrice << std::endl;
        }
    }
    if (check == false)
    {
        orderId = 0;
        orderPrice = 0;
        executedAmount = 0;
        originalAmount = 0;
    }
}

Instrument::Instrument(std::string v1name, std::string v2name,
                       BfxAPI::bitfinexAPIv2 &bfx, BfxAPI::BitfinexAPI &v1,
                       CandleInterface &iCandle,
                       const std::string jsonOrders) : _v1name(v1name), _v2name(v2name), _bfx(bfx), _apiv1(v1), _icandle(iCandle)
{
    orderId = 0;
    orderPrice = 0;
    orderSize = 0;
    originalAmount = 0;
    executedAmount = 0;
    //if (!jsonOrders.empty())
    //    initOrder(jsonOrders);
}

void Instrument::updateCandles(bool replay, const char *filepath)
{
    _candles.clear();
    if (replay == true)
    {
        _candles = std::move(_icandle.retrieveCandles(*this, filepath));
    }
    else
    {
        _candles = std::move(_icandle.retrieveCandles(*this));
    }
}

void Instrument::setOrder(std::string response, const candle &last, double totalBuy)
{
    Document document;
    document.Parse(response.c_str());
    Value &data = document;

    if (data.HasMember("message"))
    {
        logPosition({"Not able to buy: " + response});
    }
    else
    {
        std::cout << "orderId" << data["id"].GetInt64() << std::endl;

        orderId = data["id"].GetInt64();
        originalAmount = atof(data["original_amount"].GetString());
        executedAmount = atof(data["executed_amount"].GetString());
        orderPrice = last.close;
        orderSize = totalBuy;
        position = true;
    }
}

void Instrument::clearOrder()
{
    position = false;
    orderId = 0;
    orderPrice = 0;
    orderSize = 0;
    originalAmount = 0;
    executedAmount = 0;
}

void Instrument::display()
{
    std::setprecision(9);
    candle &i = _candles.back();
    time_t a = i.timestamp / 1000;
    {
        std::cout
            << std::setprecision(12) << std::fixed << "Timestamp: " << ctime(&a)
            << " Instrument: " << _v1name << " rsi: " << i.rsi
            << " Close: " << i.close << std::endl
            << " Macd: " << i.macd << std::endl
            << " Macd_signal: " << i.macdSignal << std::endl
            << " Macd_histo: " << i.macdHistogram << std::endl
            << " Hma: " << i.hma << std::endl
            << std::endl;
    }
}
