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
#include "engine.h"


// namespaces
using std::cerr;
using std::cout;
using std::endl;
using std::ifstream;
using std::string;
using namespace BfxAPI;

extern std::pair<std::string, std::string> instruments[];

int main(int argc, char *argv[])
{
    Config config(argc, argv);
    Engine engine(config);

    engine.run();

    return 0;
}
