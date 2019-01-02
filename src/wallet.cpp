#include "wallet.h"

int Wallet::update(BfxAPI::BitfinexAPI &v1)
{
    v1.getBalances();
    if (v1.hasApiError() != 0)
    {
        std::cerr << "error retrieving wallet" << std::endl;
        return (-1);
    }
    Document document;
    document.Parse(v1.strResponse().c_str());
    Value &data = document;
    std::cout << v1.strResponse() << std::endl;
    auto i = data.GetArray();

    for (auto &it : data.GetArray())
    {
        assert(it.HasMember("currency"));
        if (strcmp(it["currency"].GetString(), "btc") == 0)
        {
            assert(it.HasMember("available"));
            available = atof(it["available"].GetString());
            assert(it.HasMember("amount"));
            amount = atof(it["amount"].GetString());
        }
    }

    v1.getTicker("btcusd");
    if (v1.hasApiError() != 0)
    {
        std::cerr << "error retrieving wallet information" << std::endl;
        return (-1);
    }
    document.Parse(v1.strResponse().c_str());

    lastPrice = atof(document["last_price"].GetString());

    std::cout << "available: " << available
              << " amount: " << amount 
              << " last price: " << lastPrice << std::endl;

    std::cout << "total: " << lastPrice * available << std::endl;
    return (0);
}
