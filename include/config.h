#ifndef _CONFIG_H
#define _CONFIG_H

#include <iostream>
#include <utility>

// std
#include <fstream>
#include <iostream>
#include <iomanip>

//gap time between candles
const std::string candleGapTime("1m");
const std::string candleNumber("100");

static std::pair<std::string, std::string> instruments[] = {
    std::make_pair("trxusd", "tTRXUSD"),
    std::make_pair("xrpusd", "tXRPUSD"),
    std::make_pair("iotusd", "tIOTUSD"),
    std::make_pair("eosusd", "tEOSUSD"),
    std::make_pair("vetusd", "tVETUSD"),
    std::make_pair("dtausd", "tDTAUSD"),
    std::make_pair("cnnusd", "tCNNUSD"),
    std::make_pair("funusd", "tFUNUSD"),
    std::make_pair("xmlusd", "tXLMUSD"),
    std::make_pair("mitusd", "tMITUSD"),
    std::make_pair("intusd", "tINTUSD"),
    std::make_pair("mgousd", "tMGOUSD"),
    std::make_pair("xvgusd", "tXVGUSD"),
    std::make_pair("neousd", "tNEOUSD"),
    std::make_pair("ethusd", "tETHUSD"),
    std::make_pair("etcusd", "tETCUSD"),
    std::make_pair("zrxusd", "tZRXUSD"),
    std::make_pair("paiusd", "tPAIUSD"),
    std::make_pair("omgusd", "tOMGUSD")
    };


class Config
{
  public:
    Config(int ac, char **av);
    int init(int ac, char **av);

    bool simuMode = false;
    bool record = false;
    bool backTest = false;
    std::string accessKey;
    std::string secretKey;
    std::string replaySymbolV1;
    std::string replaySymbolV2;
};

#define NB_CANDLES 100

#endif
