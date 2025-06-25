#pragma once
#include <random>

inline float random_float(const float begin, const float end) {
    static thread_local std::mt19937 generator(std::random_device{}());
    std::uniform_real_distribution<float> distribution(begin, end);
    return distribution(generator);
}

inline int random_int(const int begin, const int end) {
    static thread_local std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<int> distribution(begin, end);
    return distribution(generator);
}