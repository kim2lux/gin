#include "config.h"
#include <stdio.h>
#include <string.h>

Config::Config(int ac, char **av)
{
    simuMode = false;
    record = false;
    backTest = false;
    init(ac, av);
}

int Config::init(int argc, char **argv)
{
    std::ifstream ifs("../doc/key-secret", std::ifstream::in);
    if (ifs.is_open())
    {
        getline(ifs, accessKey);
        getline(ifs, secretKey);
        ifs.close();
    }

    if (argc >= 2)
    {
        if (strcmp("replay", argv[1]) == 0)
        {

            simuMode = true;
            record = false;
        }
        else if (strcmp("record", argv[1]) == 0)
        {
            record = true;
            simuMode = false;
            std::cout << "recording..." << std::endl;
            return (0);
        }
        else if (strcmp("backtest", argv[1]) == 0)
        {
            backTest = true;
        }
        replaySymbolV1.clear();
        if (argc == 3)
        {
            replaySymbolV1 = argv[2];
            std::cout << "replaying instr: " << argv[2] << std::endl;
        }
        return (0);
    }
    else if (argc != 1)
    {
        simuMode = false;
        std::cout << "Command line error" << std::endl;
        exit(-1);
    }
    return (0);
}
