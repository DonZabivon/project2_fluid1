#pragma once

#include <cstdint>
#include <cassert>
#include <iostream>
#include <string>
#include <limits>

template<typename T, uint8_t N, uint8_t K, bool Fast=false> 
struct FixedT {
    typedef T type;
    static constexpr uint8_t n_size = N;
    static constexpr uint8_t k_size = K;

    static constexpr bool fast = N < sizeof(T) * 8;

    //static constexpr T inf = fast ? (1 << (N - 1)) - 1 : std::numeric_limits<T>::max();
    //static constexpr T neg_inf = ~0 & ~inf;

    T v;

    constexpr FixedT(int v) : v(fix((T)v << K)) { static_assert(N >= K); }
    constexpr FixedT(float f) : v(fix((T)(f * ((T)1 << K)))) { static_assert(N >= K); }
    constexpr FixedT(double f) : v(fix((T)(f * ((T)1 << K)))) { static_assert(N >= K); }
    constexpr FixedT() : v(0) {}

    template<typename T1, uint8_t N1, uint8_t K1, bool Fast1>
    constexpr FixedT(const FixedT<T1, N1, K1, Fast1>& rhs) : v(0) {
        //T1 int_mask = ((1 << (N1 - K1 - 1)) - 1) << K1;
        bool neg = rhs.v < 0;
        T1 x = rhs.v;
        if (neg) {
            x = -x;
        }        
        T1 int_mask1 = ((1 << (sizeof(T1) * 8 - K1 - 1)) - 1) << K1;
        T1 fract_mask1 = (1 << K1) - 1;
        
        T1 integer1 = (x & int_mask1) >> K1;
        T1 fract1 = x & fract_mask1;

        T int_mask = ((1 << (sizeof(T) * 8 - K - 1)) - 1) << K;
        T fract_mask = (1 << K) - 1;
        
        if (K > K1)
        {
            fract1 <<= (K - K1);
        } else if (K < K1) {
            fract1 >>= (K1 - K);
        }
        v = fract1 & fract_mask;
        v |= (integer1 << K) & int_mask;

        if (neg) {
            v = -v;
        }

        /*T x = v;
        if (rhs.v > 0) {
            v &= inf;
            if (v != x)
                v = inf;
        }
        if (rhs.v < 0) {
            v |= neg_inf;
            if (v != x)
                v = neg_inf;
        }*/        
    }

    static T fix(T v) {
        /*T x = v;
        if (v > 0) {
            v &= inf;
            if (v != x)
                v = inf;
        }
        if (v < 0) {
            v |= neg_inf;
            if (v != x)
                v = neg_inf;
        }*/        
        return v;
    }

    static constexpr FixedT from_raw(T x) {        
        FixedT ret;        
        ret.v = fix(x);        
        return ret;
    }  

    float to_float() const {
        return (float)v / (1LL << K);
    }

    double to_double() const {
        return (double)v / (1LL << K);
    }

    template<class RNG>
    static FixedT rand(RNG& g) {
        return FixedT::from_raw((g() & ((1LL << K) - 1)));
    }

    static constexpr std::string name() {
        std::string t;
        if (Fast) {
            t = "FAST_FIXED";
        }
        else {
            t = "FIXED";
        }
        return t + "(" + std::to_string(N) + "," + std::to_string(K) + ")";
    }

    //operator float() const { return to_float(); }
    //operator double() const { return to_double(); }

    //template<typename T1, uint8_t N1, uint8_t K1>    
    //operator FixedT<T1, N1, K1>() const;

    auto operator<=>(const FixedT&) const = default;
    bool operator==(const FixedT&) const = default;

    friend FixedT operator+(FixedT a, FixedT b) {
        return FixedT::from_raw(a.v + b.v);
    }

    friend FixedT operator-(FixedT a, FixedT b) {
        return FixedT::from_raw(a.v - b.v);
    }

    friend FixedT operator*(FixedT a, FixedT b) {
        return FixedT::from_raw(((int64_t)a.v * b.v) >> K);
    }

    friend FixedT operator/(FixedT a, FixedT b) {
        return FixedT::from_raw(((int64_t)a.v << K) / b.v);
    }

    friend FixedT& operator+=(FixedT& a, FixedT b) {
        return a = a + b;
    }

    friend FixedT& operator-=(FixedT& a, FixedT b) {
        return a = a - b;
    }

    friend FixedT& operator*=(FixedT& a, FixedT b) {
        return a = a * b;
    }

    friend FixedT& operator/=(FixedT& a, FixedT b) {
        return a = a / b;
    }

    friend FixedT operator-(FixedT x) {
        return FixedT::from_raw(-x.v);
    }

    friend FixedT abs(FixedT x) {
        if (x.v < 0) {
            x.v = -x.v;
        }
        return x;
    }

    friend std::ostream& operator<<(std::ostream& out, const FixedT& x) {
        return out << x.v / (double)(1LL << K);
    }
};

template <uint8_t N, uint8_t K>
using FastFixed8T = FixedT<int_fast8_t, N, K, true>;

template <uint8_t N, uint8_t K>
using FastFixed16T = FixedT<int_fast16_t, N, K, true>;

template <uint8_t N, uint8_t K>
using FastFixed32T = FixedT<int_fast32_t, N, K, true>;

template <uint8_t N, uint8_t K>
using FastFixed64T = FixedT<int_fast64_t, N, K, true>;

// int_fast8_t
template <uint8_t K>
using FastFixedN1 = FastFixed8T<1, K>;

template <uint8_t K>
using FastFixedN2 = FastFixed8T<2, K>;

template <uint8_t K>
using FastFixedN3 = FastFixed8T<3, K>;

template <uint8_t K>
using FastFixedN4 = FastFixed8T<4, K>;

template <uint8_t K>
using FastFixedN5 = FastFixed8T<5, K>;

template <uint8_t K>
using FastFixedN6 = FastFixed8T<6, K>;

template <uint8_t K>
using FastFixedN7 = FastFixed8T<7, K>;

template <uint8_t K>
using FastFixedN8 = FastFixed8T<8, K>;

// int_fast16_t

template <uint8_t K>
using FastFixedN9 = FastFixed16T<9, K>;

template <uint8_t K>
using FastFixedN10 = FastFixed16T<10, K>;

template <uint8_t K>
using FastFixedN11 = FastFixed16T<11, K>;

template <uint8_t K>
using FastFixedN12 = FastFixed16T<12, K>;

template <uint8_t K>
using FastFixedN13 = FastFixed16T<13, K>;

template <uint8_t K>
using FastFixedN14 = FastFixed16T<14, K>;

template <uint8_t K>
using FastFixedN15 = FastFixed16T<15, K>;

template <uint8_t K>
using FastFixedN16 = FastFixed16T<16, K>;

// int_fast32_t

template <uint8_t K>
using FastFixedN17 = FastFixed32T<17, K>;

template <uint8_t K>
using FastFixedN18 = FastFixed32T<18, K>;

template <uint8_t K>
using FastFixedN19 = FastFixed32T<19, K>;

template <uint8_t K>
using FastFixedN20 = FastFixed32T<20, K>;

template <uint8_t K>
using FastFixedN21 = FastFixed32T<21, K>;

template <uint8_t K>
using FastFixedN22 = FastFixed32T<22, K>;

template <uint8_t K>
using FastFixedN23 = FastFixed32T<23, K>;

template <uint8_t K>
using FastFixedN24 = FastFixed32T<24, K>;

template <uint8_t K>
using FastFixedN25 = FastFixed32T<25, K>;

template <uint8_t K>
using FastFixedN26 = FastFixed32T<26, K>;

template <uint8_t K>
using FastFixedN27 = FastFixed32T<27, K>;

template <uint8_t K>
using FastFixedN28 = FastFixed32T<28, K>;

template <uint8_t K>
using FastFixedN29 = FastFixed32T<29, K>;

template <uint8_t K>
using FastFixedN30 = FastFixed32T<30, K>;

template <uint8_t K>
using FastFixedN31 = FastFixed32T<31, K>;

template <uint8_t K>
using FastFixedN32 = FastFixed32T<32, K>;
