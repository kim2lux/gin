#include "engine.h"

#include <sys/types.h>
#include <dirent.h>
#include "calculate.h"

const std::string currentDateTime()
{
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

    return buf;
}

void logPosition(std::string &&str)
{

    std::ofstream file;
    file.open("orders.txt", std::ios::out | std::ios::app);
    if (file.fail())
        throw std::ios_base::failure(std::strerror(errno));

    //make sure write fails with exception if something is wrong
    file.exceptions(file.exceptions() | std::ios::failbit | std::ifstream::badbit);

    file << currentDateTime() << " - " << str.c_str() << std::endl;
}

Instrument getInstr(std::pair<std::string, std::string> &name, BfxAPI::bitfinexAPIv2 &bfxAPI,
                    BfxAPI::BitfinexAPI &v1, CandleInterface &iCandle, std::string &jsonOrders)
{

    Instrument instr(name.first, name.second, bfxAPI, v1, iCandle, jsonOrders);
    return instr;
}

int Engine::initSimuCandles(Instrument &instr)
{
    std::cout << "replaying: " << instr._v1name << std::endl;
    DIR *dirp = opendir(instr._v1name.c_str());
    struct dirent *dp;
    while ((dp = readdir(dirp)) != NULL)
    {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {
            std::string path(instr._v1name + "/" + dp->d_name);
            _simuCandles.push_back(path);
        }
    }
    closedir(dirp);
    sort(_simuCandles.begin(), _simuCandles.end());
    for (auto &path : _simuCandles)
    {
        std::cout << path << std::endl;
    }
    return (0);
}

int Engine::loadInstrument()
{
    if (_config.simuMode == false)
    {
        for (auto &i : instruments)
        {
            vInstr.push_back({i.first, i.second, _api.v2, _api.v1, _candleInterface, _api.v1.strResponse()});
        }
    }
    else
    {
        std::string order;
        for (auto &p : instruments)
        {
            if (p.first == _config.replaySymbolV1)
                vInstr.push_back({p.first, p.second, _api.v2, _api.v1, _candleInterface, order});
        }
    }
    if (vInstr.size() == 0)
    {
        std::cout << "No instrument loaded !" << std::endl;
        return (-1);
    }
    return (0);
}

int Engine::makeOrders(Instrument &instr)
{
    if (instr._candles.size() != 100)
    {
        std::cout << "Candles not loaded !" << std::endl;
        if (_config.simuMode == false)
        {
            std::cout << _api.v2.strResponse() << std::endl;
            if (_config.simuMode == false)
                sleep(10);
        }
        return (-1);
    }
    std::cout << " ****************** " << std::endl;
    _calc.updateRsi(instr);
    _calc.updateMacd(instr);
    _calc.updateHma(instr);
    instr.display();
    if (instr.orderId == 0)
        _position.makePosition(instr, _wallet, _config.simuMode);
    else
    {
        _position.shortPosition(instr, _config.simuMode);
    }
    std::cout << " ****************** " << std::endl;
    return (0);
}

int Engine::updateInstrument(Instrument &instr)
{
    if (_config.simuMode == false)
    {
        sleep(2);
        instr.updateCandles(false, nullptr);
        return (this->makeOrders(instr));
    }
    else
    {
        initSimuCandles(instr);
        for (auto &path : _simuCandles)
        {
            instr.updateCandles(true, path.c_str());
            this->makeOrders(instr);
        }
        exit(-1);
    }
    return (0);
}

void Engine::run()
{
    while (true)
    {
        if (_config.simuMode == false)
        {
            _api.v1.getActiveOrders();
            std::cout << "orders update: " << _api.v1.strResponse() << std::endl;
        }
        for (auto &instr : vInstr)
        {
            try
            {
                instr.initOrder(_api.v1.strResponse());
                if (updateInstrument(instr) != 0)
                    return;
            }
            catch (const std::runtime_error &re)
            {
                std::cerr << "Runtime error: " << re.what() << std::endl;
            }
            catch (const std::exception &ex)
            {
                std::cerr << "Error occurred: " << ex.what() << std::endl;
            }
            catch (...)
            {
                // catch any other errors (that we have no information about)
                std::cerr << "Unknown failure occurred. Possible memory corruption" << std::endl;
            }
        }
    }
}

Engine::Engine(Config &config) : _config(config), _api(config), _position(_api.v1), _candleInterface(_api.v2)
{
    if (_api.v2.getStatus() == 0)
    {
        std::cout << "Bitfinex server in maintenance" << std::endl;
        exit(0);
    }

    if (_wallet.update(_api.v1) != 0)
    {
        std::cout << "Cannot initialize wallet" << std::endl;
        exit(0);
    }
    if (loadInstrument() != 0)
    {
        std::cout << "Cannot initialize instrument" << std::endl;
        exit(0);
    }
}
