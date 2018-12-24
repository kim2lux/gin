#include "candle.h"
#include "instrument.h"
#include "config.h"

extern const std::string candleGapTime;
extern const std::string candleNumber;

std::list<candle> candleInterface::pushCandles(std::string json)
{
    Document document;
    document.Parse(json.c_str());
    Value &data = document;
    std::list<candle> candles;
    if (data.Size() != 100)
        return candles;
    for (auto &it : data.GetArray())
    {
        candles.push_front({it[0].GetUint64(),
                            it[1].GetDouble(),
                            it[2].GetDouble(),
                            it[3].GetDouble(),
                            it[4].GetDouble(),
                            it[5].GetDouble()});
    }
    return candles;
}

std::list<candle> candleInterface::retrieveCandles(instrument &instr)
{
    //_bfxApi.Request.get("/candles/trade:1m:tIOTUSD/hist?limit=100&sort=1");
    std::string request("/candles/trade:" + candleGapTime + ":" + instr._v2name + "/hist?limit=" + candleNumber + "&sort=1");
    //std::cout << "request => " << request << std::endl;
    _bfxApi.Request.get(request);
    return (pushCandles(_bfxApi.strResponse()));
}

void addLastCandle(instrument &instr, std::string json)
{
    //std::cout << json << std::endl;
    Document document;
    document.Parse(json.c_str());
    Value &data = document;
    std::cout << data.Size() << std::endl;

    instr._candles.pop_back();
    instr._candles.push_front({data[0].GetUint64(),
                               data[1].GetDouble(),
                               data[2].GetDouble(),
                               data[3].GetDouble(),
                               data[4].GetDouble(),
                               data[5].GetDouble()});
    
    std::cout << "New Candle : Open price " << data[1].GetDouble() << "Close price " << data[2].GetDouble();
}

bool candleInterface::getLastCandle(instrument &instr)
{
    std::string request("/candles/trade:" + candleGapTime + ":" + instr._v2name + "/last");
    _bfxApi.Request.get(request);
    addLastCandle(instr, _bfxApi.strResponse());
    //    pushCandles(_bfxApi.strResponse());
    return true;
}