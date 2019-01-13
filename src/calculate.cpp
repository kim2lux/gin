#include "calculate.h"
#include "indicators.h"

void Calculate::updateHma(Instrument &instr)
{
    std::array<double, NB_CANDLES> arr_x;
    int size = 0;
    for (candle &i : instr._candles)
    {
        arr_x[size] = i.close;
        size++;
    }

    TI_REAL *inputs[] = {arr_x.data()};
    TI_REAL options[] = {9}; /* period */
    TI_REAL *outputs[1];      /* hma */

    /* Determine how large the output size is for our options. */
    const int out_size =  instr._candles.size() - ti_hma_start(options);

    /* Allocate memory for output. */
    outputs[0] = (double *)malloc(sizeof(TI_REAL) * out_size);
    assert(outputs[0] != 0); /* hma */

    /* Run the actual calculation. */
    const int ret = ti_hma(instr._candles.size(), inputs, options, outputs);
    assert(ret == TI_OKAY);
    auto i = instr._candles.begin();
    auto end = instr._candles.end();
    uint32_t x = 0;
    std::advance(i, instr._candles.size() - out_size);
    while (i != end)
    {
        i->hma = outputs[0][x];
        i++;
        x++;
    }
}

void Calculate::updateMacd(Instrument &instr)
{
    std::array<double, NB_CANDLES> arr_x;
    int size = 0;
    for (candle &i : instr._candles)
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
    const int out_size = instr._candles.size() - ti_macd_start(options);

    /* Allocate memory for each output. */
    outputs[0] = (double *)malloc(sizeof(TI_REAL) * out_size);
    assert(outputs[0] != 0); /* macd */
    outputs[1] = (double *)malloc(sizeof(TI_REAL) * out_size);
    assert(outputs[1] != 0); /* macd_signal */
    outputs[2] = (double *)malloc(sizeof(TI_REAL) * out_size);
    assert(outputs[2] != 0); /* macd_histogram */

    /* Run the actual calculation. */
    const int ret = ti_macd(instr._candles.size(), inputs, options, outputs);

    auto i = instr._candles.begin();
    auto end = instr._candles.end();
    uint32_t x = 0;
    std::advance(i, instr._candles.size() - out_size);
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

void Calculate::updateRsi(Instrument &instr)
{
    std::array<double, NB_CANDLES> arr_x;
    int size = 0;
    for (candle &i : instr._candles)
    {
        arr_x[size] = i.close;
        size++;
    }

    TI_REAL *inputs[] = {arr_x.data()};
    TI_REAL options[] = {20};
    TI_REAL *outputs[1];

    const int out_size = instr._candles.size() - ti_rsi_start(options);

    outputs[0] = (double *)malloc(sizeof(TI_REAL) * out_size);
    assert(outputs[0] != 0);
    const int ret = ti_rsi(instr._candles.size(), inputs, options, outputs);

    auto i = instr._candles.begin();
    auto end = instr._candles.end();
    uint32_t x = 0;
    std::advance(i, instr._candles.size() - out_size);
    while (i != end)
    {
        i->rsi = outputs[0][x];
        i++;
        x++;
    }

    assert(ret == TI_OKAY);
}

void Calculate::updateDi(Instrument &instr)
{
    std::array<double, NB_CANDLES> arr_close;
    std::array<double, NB_CANDLES> arr_high;
    std::array<double, NB_CANDLES> arr_low;
    int size = 0;
    for (candle &i : instr._candles)
    {
        arr_high[size] = i.high;
        arr_low[size] = i.low;
        arr_close[size] = i.close;
        size++;
    }

    TI_REAL *inputs[] = {arr_high.data(), arr_low.data(), arr_close.data()};
    TI_REAL options[] = {20};
    TI_REAL *outputs[2];

    const int out_size = instr._candles.size() - ti_di_start(options);

    outputs[0] = (double *)malloc(sizeof(TI_REAL) * out_size); /* plus_di */
    assert(outputs[0] != 0);
    outputs[1] = (double *)malloc(sizeof(TI_REAL) * out_size); /* minus_di */
    assert(outputs[1] != 0);
    const int ret = ti_di(instr._candles.size(), inputs, options, outputs);
    auto i = instr._candles.begin();
    auto end = instr._candles.end();
    uint32_t x = 0;
    std::advance(i, instr._candles.size() - out_size);
    while (i != end)
    {
        i->plus_di = outputs[0][x];
        i->minus_di = outputs[1][x];
        i++;
        x++;
    }
    assert(ret == TI_OKAY);
}

void Calculate::updateAdx(Instrument &instr)
{
    std::array<double, NB_CANDLES> arr_close;
    std::array<double, NB_CANDLES> arr_high;
    std::array<double, NB_CANDLES> arr_low;
    int size = 0;
    for (candle &i : instr._candles)
    {
        arr_high[size] = i.high;
        arr_low[size] = i.low;
        arr_close[size] = i.close;
        size++;
    }

    TI_REAL *inputs[] = {arr_high.data(), arr_low.data(), arr_close.data()};
    TI_REAL options[] = {20};
    TI_REAL *outputs[1];

    const int out_size = instr._candles.size() - ti_adx_start(options);
    outputs[0] = (double *)malloc(sizeof(TI_REAL) * out_size); /* plus_di */
    assert(outputs[0] != 0);
    const int ret = ti_adx(instr._candles.size(), inputs, options, outputs);

    auto i = instr._candles.begin();
    auto end = instr._candles.end();
    uint32_t x = 0;
    std::advance(i, instr._candles.size() - out_size);
    while (i != end)
    {
        i->adx = outputs[0][x];
        i++;
        x++;
    }
    assert(ret == TI_OKAY);
}