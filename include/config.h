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
    std::make_pair("ltcbtc", "tLTCBTC"),
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
    std::make_pair("eosbtc", "tEOSBTC"),
    std::make_pair("seebtc", "tSEEBTC"),
    std::make_pair("vetbtc", "tVETBTC"),
    std::make_pair("dgbbtc", "tDGBBTC"),
    std::make_pair("funbtc", "tFUNBTC"),
    std::make_pair("xtzbtc", "tXTZBTC")};


class Config
{
  public:
    Config(int ac, char **av);
    int init(int ac, char **av);

    bool simuMode = false;
    bool record = false;
    std::string accessKey;
    std::string secretKey;
    std::string replaySymbolV1;
    std::string replaySymbolV2;
};

#define NB_CANDLES 100

#endif
