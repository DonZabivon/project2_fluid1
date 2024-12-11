#pragma once

#include <unordered_map>
#include <string>
#include <variant>

//#define TYPES FLOAT,FIXED(32,5),DOUBLE,FAST_FIXED(13,7),FIXED(32,16),FIXED(32,8)

#define FLOAT float
#define DOUBLE double
#define FIXED(N,K) FixedT<int ## N ## _t, N, K>
#define FAST_FIXED(N,K) FastFixedN ## N<K>

#ifndef TYPES
    static_assert(false, "No types specified: Use '-DTYPES=...' to provide types.");
#endif

struct TypeSet {
    static std::unordered_map<std::string, std::variant<TYPES>> types;

    template<typename T>
    static void register_one(T arg) {
        types.insert({ name<T>(), std::variant<TYPES>(arg) });
    }
};
std::unordered_map<std::string, std::variant<TYPES>> TypeSet::types;

template <typename T>
struct TypeSet1 : public TypeSet {
    static void register_all() {
        register_one<T>({});
    }
};

template <typename T1, typename T2>
struct TypeSet2 : public TypeSet {
    static void register_all() {
        register_one<T1>({});
        register_one<T2>({});
    }
};

template <typename T1, typename T2, typename T3>
struct TypeSet3 : public TypeSet {
    static void register_all() {
        register_one<T1>({});
        register_one<T2>({});
        register_one<T3>({});
    }
};

template <typename T1, typename T2, typename T3, typename T4>
struct TypeSet4 : public TypeSet {
    static void register_all() {
        register_one<T1>({});
        register_one<T2>({});
        register_one<T3>({});
        register_one<T4>({});
    }
};

template <typename T1, typename T2, typename T3, typename T4, typename T5>
struct TypeSet5 : public TypeSet {
    static void register_all() {
        register_one<T1>({});
        register_one<T2>({});
        register_one<T3>({});
        register_one<T4>({});
        register_one<T5>({});
    }
};

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
struct TypeSet6 : public TypeSet {
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
struct TypeSet7 : public TypeSet {
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
struct TypeRegistrator {
    static constexpr void register_all() {        
        if constexpr (sizeof...(Args) == 1)
            TypeSet1<Args...>::register_all();
        if constexpr (sizeof...(Args) == 2)
            TypeSet2<Args...>::register_all();
        if constexpr (sizeof...(Args) == 3)
            TypeSet3<Args...>::register_all();
        if constexpr (sizeof...(Args) == 4)
            TypeSet4<Args...>::register_all();
        if constexpr (sizeof...(Args) == 5)
            TypeSet5<Args...>::register_all();
        if constexpr (sizeof...(Args) == 6)
            TypeSet6<Args...>::register_all();
        if constexpr (sizeof...(Args) == 7)
            TypeSet7<Args...>::register_all();
    }
};