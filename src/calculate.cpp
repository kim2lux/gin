#include "calculate.h"
#include "indicators.h"

void Calculate::updateMacd(Instrument & instr)
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

    auto i = instr._candles.begin();
    auto end = instr._candles.end();
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

void Calculate::updateRsi(Instrument & instr)
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

    const int out_size = NB_CANDLES - ti_rsi_start(options);

    outputs[0] = (double *)malloc(sizeof(TI_REAL) * out_size);
    assert(outputs[0] != 0);
    const int ret = ti_rsi(NB_CANDLES, inputs, options, outputs);

    auto i = instr._candles.begin();
    auto end = instr._candles.end();
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