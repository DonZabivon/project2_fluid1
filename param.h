#pragma once

#include <vector>
#include <string>

struct Param {
    size_t n{};
    size_t m{};
    std::vector<std::vector<char>> field{};
    double g{};
    double rho_air{};
    double rho_fluid{};
    bool* stop_flag = nullptr;
};

struct Save {
    std::string name{};
    std::string p_type{};
    std::string v_type{};
    std::string f_type{};
    std::string size{};
    size_t n{};
    size_t m{};
};