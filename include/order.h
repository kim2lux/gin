#pragma once
#include "instrument.h"
#include <vector>
struct s_order {
    int64_t orderId;
    double orderPrice;
    double orderSize;
    double originalAmount;
    double executedAmount;
};

class Orders{
public:
    std::vector<s_order> _orders;
};
