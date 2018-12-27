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
#include <sys/types.h>
#include <dirent.h>
#include "calculate.h"

// namespaces
using std::cerr;
using std::cout;
using std::endl;
using std::ifstream;
using std::string;
using namespace BfxAPI;

extern std::pair<std::string, std::string> instruments[];

void replayFunc(std::vector<Instrument> &vInstr, struct wallet &w)
{
    Calculate calc;
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
            calc.updateRsi(instr);
            calc.updateMacd(instr);
            instr.display();
        }
    }
    exit(0);
}

int main(int argc, char *argv[])
{
    Config config(argc, argv);
    Engine engine(config);

    engine.run();

    return 0;
}
