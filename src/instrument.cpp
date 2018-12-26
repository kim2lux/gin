#include "instrument.h"
#include "indicators.h"
#include "config.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <time.h>

const std::string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

    return buf;
}

void instrument::updateCandles(bool replay, const char *filepath)
{
    _candles.clear();
    if (replay == true)
    {
        _candles = std::move(_icandle.retrieveCandles(*this, filepath));
    }
    else
    {
        _candles = std::move(_icandle.retrieveCandles(*this));
    }
}

void instrument::makeOrder(double totalBuy)
{
    const candle &last = _candles.back();
    std::cout << "New order: Price" << last.close << " size " << totalBuy << std::endl;
    _apiv1.newOrder(this->_v1name,
                    totalBuy * 1.0,
                    last.close / 2,
                    "buy",
                    "exchange limit",
                    false,
                    true,
                    false,
                    false,
                    0);
    if (_apiv1.hasApiError() != 0)
    {
        std::cout << "Error making the order" << std::endl;
        std::cout << _apiv1.strResponse() << std::endl;
        return;
    }
    Document document;
    document.Parse(_apiv1.strResponse().c_str());
    Value &data = document;
    std::cout << "orderId" << data["id"].GetInt64() << std::endl;

    this->orderId = data["id"].GetInt64();
    this->originalAmount = atof(data["original_amount"].GetString());
    this->executedAmount = atof(data["executed_amount"].GetString());
    this->orderPrice = last.close;
    this->orderSize = totalBuy;
    this->position = true;

    std::cout << _apiv1.strResponse() << std::endl;

    std::ofstream file;
    file.open("orders.txt", std::ios::out | std::ios::app);
    if (file.fail())
        throw std::ios_base::failure(std::strerror(errno));

    //make sure write fails with exception if something is wrong
    file.exceptions(file.exceptions() | std::ios::failbit | std::ifstream::badbit);

    file << currentDateTime() << " - New order: " << _apiv1.strResponse().c_str() << std::endl;
}

void instrument::shortOrder()
{
    const candle &last = _candles.back();
    std::cout << "selling: orderId : " << this->orderId
              << " buy price : " << this->orderPrice
              << " total size: " << this->orderSize
              << " at Price: " << last.close;
    _apiv1.newOrder(this->_v1name,
                    this->orderSize,
                    last.close,
                    "sell",
                    "exchange market",
                    false,
                    true,
                    false,
                    false,
                    0);
    Document document;
    document.Parse(_apiv1.strResponse().c_str());
    position = false;
    orderId = 0;
    orderPrice = 0;
    orderSize = 0;

    std::cout << _apiv1.strResponse() << std::endl;

    std::ofstream file;
    file.open("orders.txt", std::ios::out | std::ios::app);
    if (file.fail())
        throw std::ios_base::failure(std::strerror(errno));

    //make sure write fails with exception if something is wrong
    file.exceptions(file.exceptions() | std::ios::failbit | std::ifstream::badbit);

    file << currentDateTime() << " - Sell order: " << _apiv1.strResponse().c_str() << std::endl;
}

void instrument::display()
{
    //    candle & i =

    std::setprecision(9);
    candle &i = _candles.back();
    {
        std::cout
            << std::setprecision(12) << std::fixed
            << "Instrument: " << _v1name << " rsi: " << i.rsi
            << std::endl
            << " close: " << i.close << " rsi: " << i.rsi
            << " macd: " << i.macd
            << " macd_signal: " << i.macdSignal
            << " macd_histo: " << i.macdHistogram
            << std::endl;
    }
}

void instrument::updateMacd()
{
    std::array<double, NB_CANDLES> arr_x;
    int size = 0;
    for (candle &i : _candles)
    {
        arr_x[size] = i.close;
        size++;
    }

    /* Example usage of Moving Average Convergence/Divergence */
    /* Assuming that 'input' is a pre-loaded array of size 'in_size'. */
    TI_REAL *inputs[] = {arr_x.data()};
    TI_REAL options[] = {12, 26, 9}; /* short period, long period, signal period */
    TI_REAL *outputs[3];             /* macd, macd_signal, macd_histogram */

    /* Determine how large the output size is for our options. */
    const int out_size = NB_CANDLES - ti_macd_start(options);

    /* Allocate memory for each output. */
    outputs[0] = (double *)malloc(sizeof(TI_REAL) * out_size);
    assert(outputs[0] != 0); /* macd */
    outputs[1] = (double *)malloc(sizeof(TI_REAL) * out_size);
    assert(outputs[1] != 0); /* macd_signal */
    outputs[2] = (double *)malloc(sizeof(TI_REAL) * out_size);
    assert(outputs[2] != 0); /* macd_histogram */

    /* Run the actual calculation. */
    const int ret = ti_macd(NB_CANDLES, inputs, options, outputs);

    auto i = _candles.begin();
    auto end = _candles.end();
    uint32_t x = 0;
    std::advance(i, NB_CANDLES - out_size);
    while (i != end)
    {
        i->macd = outputs[0][x];
        i->macdSignal = outputs[1][x];
        i->macdHistogram = outputs[2][x];
        i++;
        x++;
    }
    assert(ret == TI_OKAY);
}

void instrument::updateRsi()
{
    std::array<double, NB_CANDLES> arr_x;
    int size = 0;
    for (candle &i : _candles)
    {
        arr_x[size] = i.close;
        size++;
    }

    TI_REAL *inputs[] = {arr_x.data()};
    TI_REAL options[] = {20};
    TI_REAL *outputs[1];

    const int out_size = NB_CANDLES - ti_rsi_start(options);

    outputs[0] = (double *)malloc(sizeof(TI_REAL) * out_size);
    assert(outputs[0] != 0);
    const int ret = ti_rsi(NB_CANDLES, inputs, options, outputs);

    auto i = _candles.begin();
    auto end = _candles.end();
    uint32_t x = 0;
    std::advance(i, NB_CANDLES - out_size);
    while (i != end)
    {
        i->rsi = outputs[0][x];
        i++;
        x++;
    }

    assert(ret == TI_OKAY);
}