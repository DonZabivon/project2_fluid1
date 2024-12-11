#pragma once

#include <cassert>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

#include "param.h"

bool load_data(const std::string& filename, Param &param)
{
    //static constexpr size_t N = 36;
    //static constexpr size_t M = 84;
    //static constexpr double RHO1 = 0.01;
    //static constexpr double RHO2 = 1000;
    //static constexpr double G = 0.1;    
    

    ifstream ifs(filename);
    if (!ifs) {
        return false;
    }

    string line;
    getline(ifs, line);
    istringstream iss(line);
    iss >> param.n >> param.m >> param.g >> param.rho_air >> param.rho_fluid;
    param.field = std::vector<std::vector<char>>(param.n, vector<char>(param.m + 1, 0));

    for (size_t i = 0; i < param.n; ++i)
    {
        string line;
        getline(ifs, line);
        assert(line.size() == param.m);
        for (size_t j = 0; j < param.m; ++j)
        {
            param.field[i][j] = line[j];
        }
        param.field[i][param.m] = '\0';
    }

    return true;
}

bool load_save(const std::string& filename, Param& param, Save& save)
{
    ifstream ifs(filename);
    if (!ifs) {
        return false;
    }
    
    getline(ifs, save.name);
    getline(ifs, save.p_type);
    getline(ifs, save.v_type);
    getline(ifs, save.f_type);
    getline(ifs, save.size);
    istringstream iss(save.size);
    string n, m;
    getline(iss, n, 'x');
    getline(iss, m, 'x');
    param.n = save.n = std::stoi(n);
    param.m = save.m = std::stoi(m);

    return true;
}
