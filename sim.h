#pragma once

#include <cstdint>
#include <cassert>
#include <string>
#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <random>
#include <algorithm>
#include <unordered_map>
using namespace std;

#include "fixed.h"
#include "conv.h"
#include "rand.h"
#include "param.h"
#include "data.h"
#include "ser.h"

template<typename T>
std::string name() {
    return T::name();
}

template<>
std::string name<float>() {
    return "FLOAT";
}

template<>
std::string name<double>() {
    return "DOUBLE";
}

template<typename PType, typename VType, typename FType>
std::string make_name() {
    return name<PType>() + "," + name<VType>() + "," + name<FType>();
}

// Global independent constants
static constexpr size_t DELTAS_SIZE = 4;
static constexpr std::array<pair<int, int>, DELTAS_SIZE> DELTAS{ {{-1, 0}, {1, 0}, {0, -1}, {0, 1}} };

template<typename PType, typename VType, typename FType, size_t N, size_t M, template<typename PType1, typename VType1, typename FType1, size_t N1, size_t M1> typename Storage>
class FluidSimulator {
    size_t H = N;
    size_t W = M;

    PType g{};
    std::unordered_map<char, PType> rho{};
    int UT{};
    mt19937_64 rnd{};
    bool* need_stop = nullptr;
    Storage<PType, VType, FType, N, M> S;
public:
    std::string name() const {
        std::string kind = S.is_static ? " (Static)" : " (Dynamic)";
        return make_name<PType, VType, FType>() + " " +
            std::to_string(H) + "x" + std::to_string(W) + kind;
    }

    void init(const Param& param, bool from_save = false) {
        S.init(param);
        H = param.n;
        W = param.m;

        need_stop = param.stop_flag;
        UT = 0;
        rnd.seed(1337);

        if (!from_save) {
            g = conv<double, PType>(param.g);
            rho.insert(std::make_pair(' ', conv<double, PType>(param.rho_air)));
            rho.insert(std::make_pair('.', conv<double, PType>(param.rho_fluid)));

            for (size_t x = 0; x < H; ++x) {
                for (size_t y = 0; y < W + 1; ++y) {
                    S.field[x][y] = param.field[x][y];
                }
            }

            for (size_t x = 0; x < H; ++x) {
                for (size_t y = 0; y < W; ++y) {
                    if (S.field[x][y] != '#') {
                        S.dirs[x][y] = (int)std::count_if(DELTAS.begin(), DELTAS.end(), [this, x, y](const auto& p) {
                            const auto& [dx, dy] = p;
                            return (S.field[x + dx][y + dy] != '#');
                            });
                    }
                }
            }
        }
    }

    void save(const std::string& filename) {
        std::ofstream out(filename, ios::binary);
        assert(out);

        // Text
        out << name() << endl;
        out << ::name<PType>() << endl;
        out << ::name<VType>() << endl;
        out << ::name<FType>() << endl;
        out << std::to_string(H) + "x" + std::to_string(W) << endl;

        // Binary
        write<int>(out, UT);
        write<PType>(out, g);
        for (const auto& r : rho) {
            write<char>(out, r.first);
            write<PType>(out, r.second);
        }

        for (size_t x = 0; x < H; ++x) {
            for (size_t y = 0; y < W + 1; ++y) {
                write<char>(out, S.field[x][y]);
            }
        }

        for (size_t x = 0; x < H; ++x) {
            for (size_t y = 0; y < W; ++y) {
                write<PType>(out, S.p[x][y]);
            }
        }

        for (size_t x = 0; x < H; ++x) {
            for (size_t y = 0; y < W; ++y) {
                write<PType>(out, S.old_p[x][y]);
            }
        }

        for (size_t x = 0; x < H; ++x) {
            for (size_t y = 0; y < W; ++y) {
                for (size_t z = 0; z < 4; ++z) {
                    write<VType>(out, S.velocity[x][y][z]);
                }
            }
        }

        for (size_t x = 0; x < H; ++x) {
            for (size_t y = 0; y < W; ++y) {
                for (size_t z = 0; z < 4; ++z) {
                    write<FType>(out, S.velocity_flow[x][y][z]);
                }
            }
        }

        for (size_t x = 0; x < H; ++x) {
            for (size_t y = 0; y < W; ++y) {
                write<int>(out, S.last_use[x][y]);
            }
        }

        for (size_t x = 0; x < H; ++x) {
            for (size_t y = 0; y < W; ++y) {
                write<int>(out, S.dirs[x][y]);
            }
        }
    }

    void load(const string& filename) {
        std::ifstream in(filename, ios::binary);
        assert(in);

        std::string line;

        // Text
        std::getline(in, line);//name();
        std::getline(in, line);//name<PType>();
        std::getline(in, line);//name<VType>()l;
        std::getline(in, line);//name<FType>() << endl;
        std::getline(in, line);//std::to_string(H) + "x" + std::to_string(W);


        // Binary
        UT = read<int>(in);
        g = read<PType>(in);
        char c1 = read<char>(in);
        PType rho1 = read<PType>(in);
        char c2 = read<char>(in);
        PType rho2 = read<PType>(in);
        rho.insert({ c1, rho1 });
        rho.insert({ c2, rho2 });

        for (size_t x = 0; x < H; ++x) {
            for (size_t y = 0; y < W + 1; ++y) {
                S.field[x][y] = read<char>(in);
            }
        }

        for (size_t x = 0; x < H; ++x) {
            for (size_t y = 0; y < W; ++y) {
                S.p[x][y] = read<PType>(in);
            }
        }

        for (size_t x = 0; x < H; ++x) {
            for (size_t y = 0; y < W; ++y) {
                S.old_p[x][y] = read<PType>(in);
            }
        }

        for (size_t x = 0; x < H; ++x) {
            for (size_t y = 0; y < W; ++y) {
                for (size_t z = 0; z < 4; ++z) {
                    S.velocity[x][y][z] = read<VType>(in);
                }
            }
        }

        for (size_t x = 0; x < H; ++x) {
            for (size_t y = 0; y < W; ++y) {
                for (size_t z = 0; z < 4; ++z) {
                    S.velocity_flow[x][y][z] = read<FType>(in);
                }
            }
        }

        for (size_t x = 0; x < H; ++x) {
            for (size_t y = 0; y < W; ++y) {
                S.last_use[x][y] = read<int>(in);
            }
        }

        for (size_t x = 0; x < H; ++x) {
            for (size_t y = 0; y < W; ++y) {
                S.dirs[x][y] = read<int>(in);
            }
        }
    }

    void run(size_t T)
    {
        for (size_t t = 0; t < T && !(*need_stop); ++t) {
            PType total_delta_p = 0;
            // Apply external forces
            for (size_t x = 0; x < H; ++x) {
                for (size_t y = 0; y < W; ++y) {
                    if (S.field[x][y] != '#' && S.field[x + 1][y] != '#') {
                        S.velocity[x][y][1] += conv<PType, VType>(g);
                    }
                }
            }

            // Apply forces from p
            S.move_p();
            for (int x = 0; x < (int)H; ++x) {
                for (int y = 0; y < (int)W; ++y) {
                    if (S.field[x][y] != '#') {
                        for (size_t i = 0; i < DELTAS_SIZE; ++i) {
                            const auto& [dx, dy] = DELTAS[i];
                            int nx = x + dx, ny = y + dy;
                            if (S.field[nx][ny] != '#' && S.old_p[nx][ny] < S.old_p[x][y]) {
                                PType delta_p = S.old_p[x][y] - S.old_p[nx][ny];
                                PType force = delta_p;
                                auto& contr = S.velocity[nx][ny][i % 2 ? i - 1 : i + 1];
                                PType cr = conv<VType, PType>(contr)* rho[S.field[nx][ny]];
                                if (cr < force) {
                                    force -= cr;
                                    contr = 0;
                                    S.velocity[x][y][i] += conv<PType, VType>(force / rho[S.field[x][y]]);
                                    S.p[x][y] -= force / S.dirs[x][y];
                                    total_delta_p -= force / S.dirs[x][y];
                                }
                                else {
                                    contr -= conv<PType, VType>(force / rho[S.field[nx][ny]]);
                                }
                            }
                        }
                    }
                }
            }

            // Make flow from velocities
            S.reset_flow();
            bool prop = false;
            do {
                UT += 2;
                prop = false;
                for (int x = 0; x < (int)H; ++x) {
                    for (int y = 0; y < (int)W; ++y) {
                        if (S.field[x][y] != '#' && S.last_use[x][y] != UT) {
                            auto [r, local_prop, _] = propagate_flow(x, y, 1);
                            if (r > 0) {
                                prop = true;
                            }
                        }
                    }
                }
            } while (prop);

            // Recalculate p with kinetic energy
            for (int x = 0; x < (int)H; ++x) {
                for (int y = 0; y < (int)W; ++y) {
                    if (S.field[x][y] != '#') {
                        for (size_t i = 0; i < DELTAS_SIZE; ++i) {
                            const auto& [dx, dy] = DELTAS[i];
                            PType old_v = conv<VType, PType>(S.velocity[x][y][i]);
                            PType new_v = conv<FType, PType>(S.velocity_flow[x][y][i]);
                            if (old_v > 0) {
                                assert(new_v <= old_v);
                                S.velocity[x][y][i] = conv<PType, VType>(new_v);
                                auto force = (old_v - new_v) * rho[S.field[x][y]];
                                if (S.field[x][y] == '.')
                                    force *= conv<double, PType>(0.8);
                                if (S.field[x + dx][y + dy] == '#') {
                                    S.p[x][y] += force / S.dirs[x][y];
                                    total_delta_p += force / S.dirs[x][y];
                                }
                                else {
                                    S.p[x + dx][y + dy] += force / S.dirs[x + dx][y + dy];
                                    total_delta_p += force / S.dirs[x + dx][y + dy];
                                }
                            }
                        }
                    }
                }
            }

            UT += 2;
            prop = false;
            for (int x = 0; x < (int)H; ++x) {
                for (int y = 0; y < (int)W; ++y) {
                    if (S.field[x][y] != '#' && S.last_use[x][y] != UT) {
                        if (random01<VType>(rnd) < move_prob(x, y)) {
                            prop = true;
                            propagate_move(x, y, true);
                        }
                        else {
                            propagate_stop(x, y, true);
                        }
                    }
                }
            }

            if (prop) {
                cout << "Tick " << t << ":\n";
                S.print_field();
            }
        }

        std::cout << "Saving simulation..." << endl;
        save("fluid.save");
        std::cout << "...Saved to fluid.save" << endl;
    }
private:
    tuple<FType, bool, pair<int, int>> propagate_flow(int x, int y, FType lim) {
        S.last_use[x][y] = UT - 1;
        FType ret = 0;
        for (size_t i = 0; i < DELTAS_SIZE; ++i) {
            const auto& [dx, dy] = DELTAS[i];
            int nx = x + dx, ny = y + dy;
            if (S.field[nx][ny] != '#' && S.last_use[nx][ny] < UT) {
                FType cap = conv<VType, FType>(S.velocity[x][y][i]);
                FType flow = S.velocity_flow[x][y][i];
                if (flow != cap) {
                    auto vp = min(lim, cap - flow);
                    if (S.last_use[nx][ny] == UT - 1) {
                        S.velocity_flow[x][y][i] += vp;
                        S.last_use[x][y] = UT;
                        return { vp, 1, {nx, ny} };
                    }
                    auto [t, prop, end] = propagate_flow(nx, ny, vp);
                    ret += t;
                    if (prop) {
                        S.velocity_flow[x][y][i] += t;
                        S.last_use[x][y] = UT;
                        return { t, prop && end != pair(x, y), end };
                    }
                }
            }
        }
        S.last_use[x][y] = UT;
        return { ret, 0, {0, 0} };
    }

    void propagate_stop(int x, int y, bool force = false) {
        if (!force) {
            for (size_t i = 0; i < DELTAS_SIZE; ++i) {
                const auto& [dx, dy] = DELTAS[i];
                int nx = x + dx, ny = y + dy;
                if (S.field[nx][ny] != '#' && S.last_use[nx][ny] < UT - 1 && S.velocity[x][y][i] > 0) {
                    return;
                }
            }
        }

        S.last_use[x][y] = UT;

        for (size_t i = 0; i < DELTAS_SIZE; ++i) {
            const auto& [dx, dy] = DELTAS[i];
            int nx = x + dx, ny = y + dy;
            if (S.field[nx][ny] != '#' && S.last_use[nx][ny] != UT && S.velocity[x][y][i] <= 0) {
                propagate_stop(nx, ny);
            }
        }
    }

    VType move_prob(int x, int y) {
        VType sum = 0;
        for (size_t i = 0; i < DELTAS_SIZE; ++i) {
            const auto& [dx, dy] = DELTAS[i];
            int nx = x + dx, ny = y + dy;
            if (S.field[nx][ny] != '#' && S.last_use[nx][ny] != UT) {
                auto v = S.velocity[x][y][i];
                if (v > 0) {
                    sum += v;
                }
            }
        }
        return sum;
    }

    void swap_with(int x, int y, bool renew = false) {
        static char type;
        static PType cur_p;
        static array<VType, DELTAS_SIZE> v;

        if (renew)
        {
            type = char{};
            cur_p = PType{};
            v = array<VType, DELTAS_SIZE>{};
        }

        swap(S.field[x][y], type);
        swap(S.p[x][y], cur_p);
        for (size_t i = 0; i < DELTAS_SIZE; ++i) {
            swap(S.velocity[x][y][i], v[i]);
        }
    }

    bool propagate_move(int x, int y, bool is_first) {
        S.last_use[x][y] = UT - is_first;
        bool ret = false;
        int nx = -1, ny = -1;
        do {
            std::array<VType, DELTAS_SIZE> tres;
            VType sum = 0;
            for (size_t i = 0; i < DELTAS_SIZE; ++i) {
                const auto& [dx, dy] = DELTAS[i];
                int nx = x + dx, ny = y + dy;
                if (S.field[nx][ny] != '#' && S.last_use[nx][ny] != UT) {
                    auto v = S.velocity[x][y][i];
                    if (v >= 0) {
                        sum += v;
                    }
                }
                tres[i] = sum;
            }

            if (sum == 0) {
                break;
            }

            VType p = random01<VType>(rnd) * sum;
            size_t d = std::ranges::upper_bound(tres, p) - tres.begin();

            const auto& [dx, dy] = DELTAS[d];
            nx = x + dx;
            ny = y + dy;
            assert(S.velocity[x][y][d] > 0 && S.field[nx][ny] != '#' && S.last_use[nx][ny] < UT);

            ret = (S.last_use[nx][ny] == UT - 1 || propagate_move(nx, ny, false));
        } while (!ret);

        S.last_use[x][y] = UT;

        for (size_t i = 0; i < DELTAS_SIZE; ++i) {
            const auto& [dx, dy] = DELTAS[i];
            int nx = x + dx, ny = y + dy;
            if (S.field[nx][ny] != '#' && S.last_use[nx][ny] < UT - 1 && S.velocity[x][y][i] < 0) {
                propagate_stop(nx, ny);
            }
        }

        if (ret && !is_first) {
            swap_with(x, y, true);
            swap_with(nx, ny);
            swap_with(x, y);
        }
        return ret;
    }
};

