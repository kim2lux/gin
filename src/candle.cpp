#include "candle.h"
#include "instrument.h"
#include "config.h"
#include <ctime>
#include "rapidjson/filewritestream.h"
#include <rapidjson/writer.h>
#include <cstdio>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <error.h>
using namespace rapidjson;

extern const std::string candleGapTime;
extern const std::string candleNumber;
extern std::pair<std::string, std::string> instruments[];

CandleInterface::CandleInterface(BfxAPI::bitfinexAPIv2 &bfxApi) : _bfxApi(bfxApi)
{
}

void CandleInterface::save(std::string json, std::string name)
{
    if (_record == false)
        mkdir(name.c_str(), S_IRWXU | S_IRWXG | S_IROTH);
    else
    {
        mkdir("record", S_IRWXU | S_IRWXG | S_IROTH);
        name.insert(0, "record/");
        mkdir(name.c_str(), S_IRWXU | S_IRWXG | S_IROTH);
    }
    time_t t = std::time(0);
    long int now = static_cast<long int>(t);
    Document document;
    document.Parse(json.c_str());
    std::string path(name + "/output_" + to_string(now) + ".json");
    std::cout << "saving in: " << path << std::endl;
    FILE *fp = fopen(path.c_str(), "wb"); // non-Windows use "w"
    char writeBuffer[65536];
    FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
    Writer<FileWriteStream> writer(os);
    document.Accept(writer);
    fclose(fp);
}

std::list<candle> CandleInterface::pushCandles(std::string json)
{
    Document document;
    document.Parse(json.c_str());
    Value &data = document;
    std::list<candle> candles;
    if (data.IsArray() != true)
        return candles;
    if (data.Size() <= 35)
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
    std::cout << candles.size() << std::endl;
    return (candles);
}

std::list<candle> CandleInterface::retrieveCandles(Instrument &instr, const char *filepath, uint32_t nbCandles)
{
    std::list<candle> cl;

    if (_bfxApi.Request.getLastStatusCode() != CURLE_OK)
    {
        std::cout << "error retrieving candles" << std::endl;
    }

    if (filepath != nullptr)
    {
        FILE *fp = fopen(filepath, "r"); // non-Windows use "r"
        if (fp == nullptr)
        {
            std::cout << "Error opening file" << std::endl;
            exit(-1);
        }
        char readBuffer[65536];
        FileReadStream is(fp, readBuffer, sizeof(readBuffer));
        std::string input(readBuffer);
        fclose(fp);
        if (input.find("error") != std::string::npos)
        {
            std::cout << "Error loading candle" << std::endl;
            return (cl);
        }
        return (pushCandles(input));
    }
    else
    {
        std::string nbcand(to_string(nbCandles));
        std::string request("/candles/trade:" + candleGapTime + ":t" + instr._v2name + "/hist?limit=" + nbcand + "&sort=1");
        //        std::cout << "request => " << request << std::endl;
        _bfxApi.Request.get(request);

        if (_bfxApi.Request.getLastStatusCode() != CURLE_OK)
        {
            std::cout << "error retrieving candles" << std::endl;
            sleep(10);
            return (std::list<candle>());
        }
        save(_bfxApi.strResponse(), instr._v1name);
        return (pushCandles(_bfxApi.strResponse()));
    }
}

void addLastCandle(Instrument &instr, std::string json)
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

bool CandleInterface::getLastCandle(Instrument &instr)
{
    std::string request("/candles/trade:" + candleGapTime + ":" + instr._v2name + "/last");
    _bfxApi.Request.get(request);
    addLastCandle(instr, _bfxApi.strResponse());
    //    pushCandles(_bfxApi.strResponse());
    return true;
}