////////////////////////////////////////////////////////////////////////////////
//
//  example.cpp
//
//
//  Bitfinex REST API C++ client - examples
//
////////////////////////////////////////////////////////////////////////////////

// std
#include <fstream>
#include <iostream>
#include <iomanip>

// BitfinexAPI
#include "bfx-api-cpp/BitfinexAPI.hpp"
#include "bfx-api-cpp/bfxApiV2.hpp"
#include "instrument.h"
#include "config.h"
// namespaces
using std::cerr;
using std::cout;
using std::endl;
using std::ifstream;
using std::string;
using namespace BfxAPI;

struct wallet
{
    double available = 0;
    double amount = 0;
    double lastPrice = 0;
};

std::pair<std::string, std::string> instruments[] = {
    std::make_pair("iotbtc", "tIOTBTC"),
    std::make_pair("xrpbtc", "tXRPBTC"),
    std::make_pair("xmrbtc", "tXMRBTC"),
    std::make_pair("neobtc", "tNEOBTC"),
    std::make_pair("babbtc", "tBABBTC"),
    std::make_pair("bsvbtc", "tBSVBTC"),
    std::make_pair("ethbtc", "tETHBTC"),
    std::make_pair("omgbtc", "tOMGBTC"),
    std::make_pair("xlmbtc", "tXLMBTC"),
    std::make_pair("lymbtc", "tLYMBTC"),
    std::make_pair("repbtc", "tREPBTC"),
    std::make_pair("ltcbtc", "tLTCBTC"),
    std::make_pair("eosbtc", "tEOSBTC"),
    std::make_pair("xtzbtc", "tXTZBTC")};

instrument getInstr(std::pair<std::string, std::string> &name, bitfinexAPIv2 &bfxAPI,
                    BfxAPI::BitfinexAPI &v1, candleInterface &iCandle)
{
    if (bfxAPI.getStatus() == 0)
    {
        std::cout << "Bitfinex server in maintenance" << std::endl;
        exit(0);
    }
    instrument instr(name.first, name.second, bfxAPI, v1, iCandle);
    return instr;
}

void shortPosition(instrument &i)
{
    const candle &last = i._candles.back();

    if (last.rsi > 65)
    {
        std::cout << "RSI high" << std::endl;
        if (last.macdSignal > last.macd)
        {
            std::cout << "Shorting position" << std::endl;
            i.shortOrder();
            i.orderId = 0;
            i.orderPrice = 0;
        }
        if (last.rsi > 78)
        {
            i.shortOrder();
            i.orderId = 0;
            i.orderPrice = 0;
        }
        else
        {
            std::cout << "MACD: Price still increasing" << std::endl;
        }
    }
    else
    {
        std::cout << "rsi too low to sell" << std::endl;
    }
}

void makePosition(instrument &i, struct wallet & w)
{
    const candle &last = i._candles.back();
    double value = 10 / w.lastPrice;

    double totalBuy = value / last.close;

    std::cout << "Value to buy: " << totalBuy << std::endl;

    if (last.rsi < 27)
    {
        if (last.macd > last.macdSignal)
        {
            std::cout << "Buying position" << std::endl;
            i.makeOrder(totalBuy);
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
}

void getWallet(BfxAPI::BitfinexAPI &v1, struct wallet & w)
{
    v1.getBalances();
    //    std::cout << v1.strResponse() << std::endl;
    Document document;
    document.Parse(v1.strResponse().c_str());
    Value &data = document;
    std::cout << v1.strResponse() << std::endl;
    auto i = data.GetArray();

    for (auto &it : data.GetArray())
    {
        assert(it.HasMember("currency"));
        if (strcmp(it["currency"].GetString(), "btc") == 0)
        {
            assert(it.HasMember("available"));
            w.available = atof(it["available"].GetString());
            assert(it.HasMember("amount"));
            w.amount = atof(it["amount"].GetString());
        }
    }

    v1.getTicker("btcusd");
    std::cout << v1.strResponse() << std::endl;
    document.Parse(v1.strResponse().c_str());

    std::cout << "last price: " << document["last_price"].GetString() << std::endl;
    w.lastPrice = atof(document["last_price"].GetString());

    std::cout << w.available << " amount " << w.amount << " last price " << w.lastPrice << std::endl;

    std::cout << w.lastPrice * w.available << std::endl;
}

int main(int argc, char *argv[])
{
    string accessKey;
    string secretKey;
    struct wallet w;

    std::vector<instrument> symbols;
    ifstream ifs("../doc/key-secret", ifstream::in);
    if (ifs.is_open())
    {
        getline(ifs, accessKey);
        getline(ifs, secretKey);
        ifs.close();
        //      bfxAPI.setKeys(accessKey, secretKey);
    }
    std::cout << "Init Api" << std::endl;
    BfxAPI::bitfinexAPIv2 bfxAPI(accessKey, secretKey);
    candleInterface iCandle(candleGapTime, bfxAPI);
    BfxAPI::BitfinexAPI v1(accessKey, secretKey);

    std::vector<instrument> vInstr;
    for (auto &i : instruments)
    {
        vInstr.push_back(std::move(getInstr(i, bfxAPI, v1, iCandle)));
    }

    getWallet(v1, w);

    while (true)
    {
        for (auto &instr : vInstr)
        {
            sleep(3);
            instr.updateCandles();
            if (instr._candles.size() != 100) {
                std::cout << "Candles not loaded !" << std::endl;
                std::cout << bfxAPI.strResponse() << std::endl;
                sleep(10);
                break;
            }
            instr.updateRsi();
            instr.updateMacd();
            instr.display();
            if (instr.orderId == 0)
                makePosition(instr, w);
            else
                shortPosition(instr);
        }
    }

    return 0;
}
