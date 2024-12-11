#pragma once

#include <random>
using namespace std;

namespace {
    static std::uniform_real_distribution<float> FLOAT_DISTR = std::uniform_real_distribution<float>(0.0, 1.0);
	static std::uniform_real_distribution<double> DOUBLE_DISTR = std::uniform_real_distribution<double>(0.0, 1.0);

    template<typename T, class RNG>
    struct Random01 {
        T operator()(RNG& g) {
            return T::rand(g);
        }
    };

    template<class RNG>
    struct Random01<float, RNG> {
        float operator()(RNG& g) {
            return FLOAT_DISTR(g);
        }
    };

    template<class RNG>
    struct Random01<double, RNG> {
        double operator()(RNG& g) {
            return DOUBLE_DISTR(g);
        }
    };
}

template<typename T, class RNG>
T random01(RNG& g) {
    return Random01<T, RNG>()(g);
}