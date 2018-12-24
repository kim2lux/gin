#pragma once

class ticker {
    public:
        std::string instrument;
        float       bid;
        float bid_size;
        float ask;
        float ask_size;
        float daily_change;
        float daily_change_perc;
        float last_price;
        float volume;
        float high;
        float low;
};
