#pragma once

#include <unordered_map>
#include <string>
#include <variant>

//#define SIZES S(36,84),S(1920,1080),S(10,10),S(42,1337),

#define S(N,M) Size<N,M>

#ifndef SIZES
    #define SIZES S(0,0)
#endif

template <size_t N, size_t M>
struct Size {
    static constexpr size_t n = N;
    static constexpr size_t m = M;
    static constexpr std::string name() {
        return std::to_string(N) + "x" + std::to_string(M);
    }
};

struct SizeSet {
    static std::unordered_map<std::string, std::variant<SIZES>> sizes;

    template<typename T>
    static void register_one(T arg) {
        if (T::n != 0 && T::m != 0)
            sizes.insert({ T::name(), std::variant<SIZES>(arg) });
    }
};
std::unordered_map<std::string, std::variant<SIZES>> SizeSet::sizes;

template <typename T>
struct SizeSet1 : public SizeSet {
    static void register_all() {
        register_one<T>({});
    }
};

template <typename T1, typename T2>
struct SizeSet2 : public SizeSet {
    static void register_all() {
        register_one<T1>({});
        register_one<T2>({});        
    }
};

template <typename T1, typename T2, typename T3>
struct SizeSet3 : public SizeSet {
    static void register_all() {
        register_one<T1>({});
        register_one<T2>({});
        register_one<T3>({});        
    }
};

template <typename T1, typename T2, typename T3, typename T4>
struct SizeSet4 : public SizeSet {
    static void register_all() {
        register_one<T1>({});
        register_one<T2>({});
        register_one<T3>({});
        register_one<T4>({});        
    }
};

template <typename T1, typename T2, typename T3, typename T4, typename T5>
struct SizeSet5 : public SizeSet {
    static void register_all() {
        register_one<T1>({});
        register_one<T2>({});
        register_one<T3>({});
        register_one<T4>({});
        register_one<T5>({});
    }
};

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
struct SizeSet6 : public SizeSet {
    static void register_all() {
        register_one<T1>({});
        register_one<T2>({});
        register_one<T3>({});
        register_one<T4>({});
        register_one<T5>({});
        register_one<T6>({});       
    }
};

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
struct SizeSet7 : public SizeSet {
    static void register_all() {
        register_one<T1>({});
        register_one<T2>({});
        register_one<T3>({});
        register_one<T4>({});
        register_one<T5>({});
        register_one<T6>({});
        register_one<T7>({});
    }
};

template<typename ...Args>
struct SizeRegistrator {
    static auto register_all() {
        if constexpr (sizeof...(Args) == 1)
            SizeSet1<Args...>::register_all();
        if constexpr (sizeof...(Args) == 2)
            SizeSet2<Args...>::register_all();
        if constexpr (sizeof...(Args) == 3)
            SizeSet3<Args...>::register_all();
        if constexpr (sizeof...(Args) == 4)
            SizeSet4<Args...>::register_all();
        if constexpr (sizeof...(Args) == 5)
            SizeSet5<Args...>::register_all();
        if constexpr (sizeof...(Args) == 6)
            SizeSet6<Args...>::register_all();
        if constexpr (sizeof...(Args) == 7)
            SizeSet7<Args...>::register_all();
    }
};