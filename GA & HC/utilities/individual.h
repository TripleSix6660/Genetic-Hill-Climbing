#pragma once
#include <vector>

struct individual {
    std::vector<uint8_t> genome;
    float fitness = -1e9f;
};