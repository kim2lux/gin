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

void save(std::string json, std::string &name)
{
    if (mkdir(name.c_str(), S_IRWXU | S_IRWXG | S_IROTH))
    {
        std::cout << "hm" << std::endl;
    }
    time_t t = std::time(0);
    long int now = static_cast<long int>(t);
    Document document;
    document.Parse(json.c_str());
    std::string path(name + "/output_" + to_string(now) + ".json");
    FILE *fp = fopen(path.c_str(), "wb"); // non-Windows use "w"
    char writeBuffer[65536];
    FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
    Writer<FileWriteStream> writer(os);
    document.Accept(writer);
    fclose(fp);
}

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

std::list<candle> candleInterface::retrieveCandles(instrument &instr, const char *filepath)
{
    if (filepath != nullptr)
    {
        FILE *fp = fopen(filepath, "r"); // non-Windows use "r"
        if (fp == nullptr) 
        {
            std::cout << "Error opening file" << std::endl;
            exit (-1);
        }
        char readBuffer[65536];
        FileReadStream is(fp, readBuffer, sizeof(readBuffer));
        std::string input(readBuffer);
        fclose(fp);
        return (pushCandles(input));
    }
    else
    {
        std::string request("/candles/trade:" + candleGapTime + ":" + instr._v2name + "/hist?limit=" + candleNumber + "&sort=1");
        //std::cout << "request => " << request << std::endl;
        _bfxApi.Request.get(request);
        save(_bfxApi.strResponse(), instr._v1name);
        return (pushCandles(_bfxApi.strResponse()));
    }
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