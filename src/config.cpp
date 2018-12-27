#include "config.h"

Config::Config(int ac, char **av)
{
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

    if (argc == 4)
    {
        std::cout << "Init Api" << std::endl;

        std::cout << argc << std::endl;
        if (argc == 4)
        {
            simuMode = true;
            replaySymbolV1 = argv[2];
            replaySymbolV2 = argv[3];
            std::cout << "replaying instr: " << argv[2] << argv[3] << std::endl;
        }
        else if (argc != 1)
        {
            simuMode = false;
            std::cout << "Command line error" << std::endl;
        }
    }
    return (0);
}
