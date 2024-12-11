#pragma once

#include <algorithm>
#include <vector>
#include <string>

#include "param.h"

template<typename PType, typename VType, typename FType, size_t N, size_t M>
struct SimData {
    char field[N][M + 1]{};
    PType p[N][M]{};
    PType old_p[N][M]{};
    VType velocity[N][M][4]{};
    FType velocity_flow[N][M][4]{};
    int last_use[N][M]{};
    int dirs[N][M]{};

    static constexpr bool is_static = true;

    void init(const Param& param) {

    }

    void reset_flow() {
        for (size_t x = 0; x < N; ++x) {
            for (size_t y = 0; y < M; ++y) {
                for (size_t z = 0; z < 4; ++z) {
                    velocity_flow[x][y][z] = 0;
                }
            }
        }
    }
    void move_p() {
        memcpy(old_p, p, sizeof(p));
    }

    void print_field() {
        for (size_t x = 0; x < N; ++x) {
            cout << field[x] << "\n";
        }
    }
};

template<typename PType, typename VType, typename FType>
struct SimData<PType, VType, FType, 0, 0> {
    size_t H{};
    size_t W{};    

    vector<vector<char>> field{};
    vector<vector<PType>> p{};
    vector<vector<PType>> old_p{};
    vector<vector<vector<VType>>> velocity{};
    vector<vector<vector<FType>>> velocity_flow{};
    vector<vector<int>> last_use{};
    vector<vector<int>> dirs{};

    static constexpr bool is_static = false;

    void init(const Param& param) {
        H = param.n;
        W = param.m;

        field = vector<vector<char>>(H, vector<char>(W + 1, 0));
        p = vector<vector<PType>>(H, vector<PType>(W, 0));
        old_p = vector<vector<PType>>(H, vector<PType>(W, 0));
        velocity = vector<vector<vector<VType>>>(H, vector<vector<VType>>(W, vector<VType>(4, 0)));
        velocity_flow = vector<vector<vector<FType>>>(H, vector<vector<FType>>(W, vector<FType>(4, 0)));
        last_use = vector<vector<int>>(H, vector<int>(W, 0));
        dirs = vector<vector<int>>(H, vector<int>(W, 0));        
    }

    void reset_flow() {
        for (auto& row : velocity_flow) {
            for (auto& col : row) {
                for (auto& val : col) {
                    val = 0;
                }
            }
        }
    }

    void move_p() {
        for (size_t x = 0; x < H; ++x) {
            for (size_t y = 0; y < W; ++y) {
                old_p[x][y] = p[x][y];
            }
        }
    }

    void print_field() {
        for (size_t x = 0; x < H; ++x) {
            cout << &field[x][0] << "\n";
        }
    }
};


