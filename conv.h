#pragma once

// Type conversion

namespace {
    template<typename From, typename To>
    struct Converter {
        To operator()(const From& v1) {
            return (To)(v1);
        }
    };

    template<typename From>
    struct Converter<From, float> {
        float operator()(const From& v1) {
            return v1.to_float();
        }
    };

    template<typename From>
    struct Converter<From, double> {
        double operator()(const From& v1) {
            return v1.to_double();
        }
    };

    template<>
    struct Converter<float, double> {
        double operator()(const float& v1) {
            return (double)v1;
        }
    };

    template<>
    struct Converter<double, double> {
        double operator()(const double& v1) {
            return (double)v1;
        }
    };

    template<>
    struct Converter<double, float> {
        float operator()(const double& v1) {
            return (float)v1;
        }
    };

    template<>
    struct Converter<float, float> {
        float operator()(const float& v1) {
            return (float)v1;
        }
    };
}

template<typename From, typename To>
To conv(const From& v) {
    return Converter<From, To>()(v);
}