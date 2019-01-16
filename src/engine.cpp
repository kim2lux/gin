#include "engine.h"

#include <sys/types.h>
#include <dirent.h>
#include "calculate.h"
extern double totalDiff;
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

    file << std::endl
         << str.c_str() << std::endl;
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
    std::string record("record/" + instr._v1name);
    DIR *dirp = opendir(record.c_str());
    struct dirent *dp;
    _simuCandles.clear();
    while ((dp = readdir(dirp)) != NULL)
    {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {
            std::string path(record + "/" + dp->d_name);
            _simuCandles.push_back(path);
        }
    }
    closedir(dirp);
    sort(_simuCandles.begin(), _simuCandles.end());
    return (0);
}

std::string convertSymToV2(std::string v1)
{
    std::string v2(7, 0);
    std::transform(v1.begin(), v1.end(), v2.begin(), ::toupper);
    const char *str = v2.c_str();
    std::string res(str);
    return res;
}

int Engine::retrieveInstrument()
{
    //    _instruments.push_back(make_pair(std::string("sntbtc"), std::string("SNTBTC")));
    //    return (0);
    _api.v1.getSymbols();

    while (_api.v1.hasApiError() != 0)
    {
        sleep(2);
        std::cout << "Not able to retrieve symbol... retry..." << std::endl;
        _api.v1.getSymbols();
    }

    Document document;
    document.Parse(_api.v1.strResponse().c_str());
    Value &data = document;
    if (data.Size() < 150)
        return -1;
    for (uint32_t ite = 0; ite < data.Size(); ite += 1)
    {
        std::string v1 = data[ite].GetString();
        std::string v2;
        const std::string suffix("usd");
        if (v1.compare(3, 3, suffix) == 0)
        {
            v2 = convertSymToV2(data[ite].GetString());
            //std::cout << data[ite].GetString() << std::endl;
            _instruments.push_back(make_pair(v1, v2));
        }

        //            _instruments.push_back(data[ite].GetString());
    }
    return 0;
}

int Engine::isVolume(std::string v1name, int minVolume)
{
    sleep(3);
    std::cout << "retrieve volume: " << v1name << std::endl;
    _api.v1.getTicker(v1name);
    if (_api.v1.hasApiError())
    {
        std::cout << "Error retrieving volume" << _api.v1.strResponse() << std::endl;
        return (-1);
    }
    else
    {
        std::cout << _api.v1.strResponse() << std::endl;
        Document document;
        document.Parse(_api.v1.strResponse().c_str());
        Value &data = document;
        double volume = atof(data["volume"].GetString());
        std::cout << "instr: " << v1name << " volume: " << volume << std::endl;
        if (volume > minVolume)
            return (0);
    }
    return (-1);
}

int Engine::loadInstrument()
{
    retrieveInstrument();
    //isVolume(5000);
    if (_config.simuMode == false)
    {
        for (auto &i : _instruments)
        {
            std::cout << i.first << std::endl;
            //
            vInstr.push_back({i.first, i.second, _api.v2, _api.v1, _candleInterface, _api.v1.strResponse()});
        }
    }
    else
    {
        std::string order;
        for (auto &p : _instruments)
        {
            if (_config.replaySymbolV1.size() > 1)
            {
                if (p.first == _config.replaySymbolV1)
                    vInstr.push_back({p.first, p.second, _api.v2, _api.v1, _candleInterface, order});
            }
            else
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
    candle &last = instr._candles.back();

    time_t now = time(0);

    uint64_t seconds = difftime(now, 0);
    uint64_t lastCandle = last.timestamp / 1000;

    if (lastCandle + 36000 < seconds && _config.backTest == false) {
        std::cout << instr._v1name << ": Candle too old" << std::endl;
        return (-1);
    }

    if (instr._candles.size() <= 25)
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
    else
    {
        std::cout << " ****************** " << std::endl;
        _calc.updateRsi(instr);
        _calc.updateMacd(instr);
        _calc.updateHma(instr);
        _calc.updateDi(instr);
        _calc.updateAdx(instr);
        instr.display();
        if (instr.orderBuyId == 0 && instr.orderSellId == 0)
            _position.makePosition(instr, _wallet, _config.simuMode, _config.backTest);
        else
        {
            _position.shortPosition(instr, _wallet, _config.simuMode, _config.backTest);
        }
        std::cout << " ****************** " << std::endl;
    }
    return (0);
}

int Engine::updateInstrument(Instrument &instr)
{
    if (_config.simuMode == false && _config.backTest == false)
    {
        usleep(2000 * 1000);
        instr.updateCandles(false, nullptr);
        if (_config.record == false)
            return (this->makeOrders(instr));
    }
    else if (_config.backTest == true)
    {

        instr._totalCandles = std::move(_candleInterface.retrieveCandles(instr, nullptr, 5000));
        std::cout << "total candle loaded" << std::endl;
        if (instr._totalCandles.size() < 500)
        {
            std::cout << "total candle too small" << std::endl;
            sleep(10);
            return (0);
        }
        instr.loadCandleFromAll();
        while (instr._totalCandles.size() > instr._candlePosition)
        {
            instr.updateFromAll();
            this->makeOrders(instr);
        }
        sleep(15);
    }
    else if (_config.simuMode == true)
    {
        initSimuCandles(instr);
        for (auto &path : _simuCandles)
        {
            instr.updateCandles(true, path.c_str());
            this->makeOrders(instr);
        }
    }
    return (0);
}

void Engine::run()
{

    if (_config.simuMode == false && _config.record == false && _config.backTest == false)
    {
        _api.v1.getActiveOrders();
        if (_api.v1.Request.getLastStatusCode() != CURLE_OK)
        {
            std::cout << "error retrieve active order" << std::endl;
            return;
        }
        if (_api.v1.hasApiError())
        {
            std::cout << "orders api error" << std::endl;
            return;
        }
        for (auto &instr : vInstr)
        {
            instr.clearOrder();
            instr.initOrder(_api.v1.strResponse());
        }
    }
    for (auto &instr : vInstr)
    {
        instr.backTest = _config.backTest;
        try
        {
            if (updateInstrument(instr) != 0)
                std::cout << "issue retrieving candles" << std::endl;
        }
        catch (const std::runtime_error &re)
        {
            std::cout << "Runtime error: " << re.what() << std::endl;
            return;
        }
        catch (const std::exception &ex)
        {
            std::cout << "Error occurred: " << ex.what() << std::endl;
            return;
        }
        catch (...)
        {
            std::cout << "Unknown failure occurred" << std::endl;
            return;
        }
        std::cout << std::fixed << std::setprecision(9) << "Total: " << totalDiff << std::endl;
    }
    if (_config.simuMode == true || _config.backTest == true)
        exit(0);
}

Engine::Engine(Config &config) : _config(config), _api(config), _position(_api.v1), _candleInterface(_api.v2)
{
    _candleInterface._record = _config.record;
    if (_api.v2.getStatus() == 0)
    {
        std::cout << "Bitfinex server in maintenance" << std::endl;
        exit(0);
    }

    while (_wallet.update(_api.v1) != 0)
    {
        std::cout << "Cannot initialize wallet" << std::endl;
        sleep(10);
    }
    while (loadInstrument() != 0)
    {
        std::cout << "Cannot initialize instrument" << std::endl;
        sleep(10);
    }
}
