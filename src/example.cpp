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
#include <sys/types.h>
#include <dirent.h>

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
    std::make_pair("dshbtc", "tDSHBTC"),
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

int isMacdReducing(instrument &i, bool sell = false)
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

int shortPosition(instrument &i)
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
            i.shortOrder();
            i.orderId = 0;
            i.orderPrice = 0;
            return (0);
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
    return (-1);
}

void makePosition(instrument &i, struct wallet &w)
{
    const candle &last = i._candles.back();
    double unitPrice = last.close * w.lastPrice;

    double totalBuy = 20 / unitPrice;

    std::cout << "Value to buy: " << totalBuy << std::endl;
    std::cout << "Price to buy: " << totalBuy * unitPrice << std::endl;

    if (last.rsi < 32)
    {
        if (isMacdReducing(i, false) == 0)
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

void getWallet(BfxAPI::BitfinexAPI &v1, struct wallet &w)
{
    v1.getBalances();
    if (v1.hasApiError() != 0)
    {
        std::cerr << "error retrieving wallet" << std::endl;
        return;
    }
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
    if (v1.hasApiError() != 0)
    {
        std::cerr << "error retrieving ticker" << std::endl;
        return;
    }
    std::cout << v1.strResponse() << std::endl;
    document.Parse(v1.strResponse().c_str());

    std::cout << "last price: " << document["last_price"].GetString() << std::endl;
    w.lastPrice = atof(document["last_price"].GetString());

    std::cout << w.available << " amount " << w.amount << " last price " << w.lastPrice << std::endl;

    std::cout << w.lastPrice * w.available << std::endl;
}

void replayFunc(std::vector<instrument> &vInstr, struct wallet &w)
{
    std::vector<std::string> filepathVector;
    for (auto &instr : vInstr)
    {
        DIR *dirp = opendir(instr._v1name.c_str());
        struct dirent *dp;
        while ((dp = readdir(dirp)) != NULL)
        {
            if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
            {
                std::string path(instr._v1name + "/" + dp->d_name);
                filepathVector.push_back(path);
            }
        }
        closedir(dirp);
        sort(filepathVector.begin(), filepathVector.end());
        for (auto &path : filepathVector)
        {
            std::cout << path << std::endl;
        }
        for (auto &path : filepathVector)
        {
            std::cout << path << std::endl;

            instr.updateCandles(true, path.c_str());
            if (instr._candles.size() != 100)
            {
                std::cout << "Candles not loaded !" << std::endl;
                break;
            }
            instr.updateRsi();
            instr.updateMacd();
            instr.display();
        }
    }
    exit(0);
}

int main(int argc, char *argv[])
{
    string accessKey;
    string secretKey;
    struct wallet w;
    bool replay;
    std::cout << "Init Api" << std::endl;

    std::cout << argc << std::endl;
    if (argc == 4)
    {
        replay = true;
        std::cout << "replaying instr: " << argv[2] << argv[3] << std::endl;
    }
    else if (argc != 1)
    {
        std::cout << "Command line error" << std::endl;
    }

    std::vector<instrument> symbols;
    ifstream ifs("../doc/key-secret", ifstream::in);
    if (ifs.is_open())
    {
        getline(ifs, accessKey);
        getline(ifs, secretKey);
        ifs.close();
        //      bfxAPI.setKeys(accessKey, secretKey);
    }

    BfxAPI::bitfinexAPIv2 bfxAPI(accessKey, secretKey);
    candleInterface iCandle(candleGapTime, bfxAPI);
    BfxAPI::BitfinexAPI v1(accessKey, secretKey);
    std::vector<instrument> vInstr;

    getWallet(v1, w);

    if (replay == false)
    {

        for (auto &i : instruments)
        {
            vInstr.push_back(std::move(getInstr(i, bfxAPI, v1, iCandle)));
        }
    }
    else
    {
        std::pair<std::string, std::string> p = std::make_pair(argv[2], argv[3]);
        vInstr.push_back(std::move(getInstr(p, bfxAPI, v1, iCandle)));
        replayFunc(vInstr, w);
    }

    while (true)
    {
        for (auto &instr : vInstr)
        {
            sleep(2);
            instr.updateCandles(false, nullptr);
            if (instr._candles.size() != 100)
            {
                std::cout << "Candles not loaded !" << std::endl;
                std::cout << bfxAPI.strResponse() << std::endl;
                sleep(20);
                break;
            }
            instr.updateRsi();
            instr.updateMacd();
            instr.display();
            if (instr.orderId == 0)
                makePosition(instr, w);
            else
            {
                while (shortPosition(instr) != 0)
                {
                    sleep(2);
                    instr.updateRsi();
                    instr.updateMacd();
                    instr.display();
                }
                exit(0);
            }
        }
    }

    return 0;
}
