#include <iostream>
#include <string>
#include <csignal>
#include <cstdlib>
#include <cstring>
using namespace std;

#include "sim.h"
#include "param.h"
#include "data_loader.h"
#include "size_set.h"
#include "type_set.h"


bool stop_flag = false;

template<typename PType, typename VType, typename FType, size_t N, size_t M>
struct SimRunner {    
    void operator()(const Param& param) {
        FluidSimulator<PType, VType, FType, N, M, SimData> sim;
        sim.init(param);

        std::cout << "Starting Simulator with parameters " << sim.name() << std::endl;

        size_t T = 1'000'000;
        sim.run(T);        
    }
};

template<typename PType, typename VType, typename FType, size_t N, size_t M>
struct SimLoader {
    void operator()(const Param& param, const string& save_filename) {
        FluidSimulator<PType, VType, FType, N, M, SimData> sim;
        sim.init(param, true);
        sim.load(save_filename);
        std::cout << "Starting Simulator with parameters " << sim.name() << std::endl;

        size_t T = 1'000'000;
        sim.run(T);
    }
};


auto run_dynamic_sim(const Param& param, std::string p_type, std::string v_type, std::string f_type) {    
    if (TypeSet::types.count(p_type) > 0 && TypeSet::types.count(v_type) > 0 && TypeSet::types.count(f_type) > 0) {
        auto& v1 = TypeSet::types[p_type];
        auto& v2 = TypeSet::types[v_type];
        auto& v3 = TypeSet::types[f_type];

        visit([&param, &v2, &v3](auto&& arg) {
            using T1 = std::decay_t<decltype(arg)>;
            visit([&param, &v3](auto&& arg) {
                using T2 = std::decay_t<decltype(arg)>;
                visit([&param](auto&& arg) {
                    using T3 = std::decay_t<decltype(arg)>;                    
                    SimRunner<T1, T2, T3, 0, 0>()(param);
                    }, v3);
                }, v2);
            }, v1);
    } else {
        cerr << "Provided types was not registered." << endl;
    }
}

auto run_static_sim(const Param& param, std::string p_type, std::string v_type, std::string f_type, std::string size) {    
    if (TypeSet::types.count(p_type) > 0 && TypeSet::types.count(v_type) > 0 && TypeSet::types.count(f_type) > 0) {
        auto& v1 = TypeSet::types[p_type];
        auto& v2 = TypeSet::types[v_type];
        auto& v3 = TypeSet::types[f_type];
        auto& v4 = SizeSet::sizes[size];

        visit([&param, &v2, &v3, &v4](auto&& arg) {
            using T1 = std::decay_t<decltype(arg)>;
            visit([&param, &v3, &v4](auto&& arg) {
                using T2 = std::decay_t<decltype(arg)>;
                visit([&param, &v4](auto&& arg) {
                    using T3 = std::decay_t<decltype(arg)>;
                    visit([&param](auto&& arg) {
                        using T4 = std::decay_t<decltype(arg)>;                        
                        SimRunner<T1, T2, T3, T4::n, T4::m>()(param);
                        }, v4);
                    }, v3);
                }, v2);
            }, v1);
    } else {
        cerr << "Provided types was not registered." << endl;
    }    
}

auto restore_dynamic_sim(const string& fn, const Param& param, std::string p_type, std::string v_type, std::string f_type) {
    if (TypeSet::types.count(p_type) > 0 && TypeSet::types.count(v_type) > 0 && TypeSet::types.count(f_type) > 0) {
        auto& v1 = TypeSet::types[p_type];
        auto& v2 = TypeSet::types[v_type];
        auto& v3 = TypeSet::types[f_type];

        visit([&fn, &param, &v2, &v3](auto&& arg) {
            using T1 = std::decay_t<decltype(arg)>;
            visit([&fn, &param, &v3](auto&& arg) {
                using T2 = std::decay_t<decltype(arg)>;
                visit([&fn, &param](auto&& arg) {
                    using T3 = std::decay_t<decltype(arg)>;
                    SimLoader<T1, T2, T3, 0, 0>()(param, fn);
                    }, v3);
                }, v2);
            }, v1);
    }
    else {
        cerr << "Provided types was not registered." << endl;
    }
}

auto restore_static_sim(const string& fn, const Param& param, std::string p_type, std::string v_type, std::string f_type, std::string size) {
    if (TypeSet::types.count(p_type) > 0 && TypeSet::types.count(v_type) > 0 && TypeSet::types.count(f_type) > 0) {
        auto& v1 = TypeSet::types[p_type];
        auto& v2 = TypeSet::types[v_type];
        auto& v3 = TypeSet::types[f_type];
        auto& v4 = SizeSet::sizes[size];

        visit([&fn, &param, &v2, &v3, &v4](auto&& arg) {
            using T1 = std::decay_t<decltype(arg)>;
            visit([&fn, &param, &v3, &v4](auto&& arg) {
                using T2 = std::decay_t<decltype(arg)>;
                visit([&fn, &param, &v4](auto&& arg) {
                    using T3 = std::decay_t<decltype(arg)>;
                    visit([&fn, &param](auto&& arg) {
                        using T4 = std::decay_t<decltype(arg)>;
                        SimLoader<T1, T2, T3, T4::n, T4::m>()(param, fn);
                        }, v4);
                    }, v3);
                }, v2);
            }, v1);
    }
    else {
        cerr << "Provided types was not registered." << endl;
    }

}

void sigint_handler(int sig) {
    stop_flag = true;
}

#define P_OPT "--p-type="
#define V_OPT "--v-type="
#define F_OPT "--v-flow-type="
#define I_OPT "--input="
#define L_OPT "--load="

int main(int argc, char *argv[])
{    
    //size_t N = 36, M = 84;
    std::string p_type;
    std::string v_type;
    std::string f_type;
    std::string input;
    std::string load_from;

    for (int i = 1; i < argc; ++i) {
        if (strncmp(argv[i], P_OPT, strlen(P_OPT)) == 0) {
            p_type = std::string(argv[i]).substr(strlen(P_OPT));
        } else if (strncmp(argv[i], V_OPT, strlen(V_OPT)) == 0) {
            v_type = std::string(argv[i]).substr(strlen(V_OPT));
        } else if (strncmp(argv[i], F_OPT, strlen(F_OPT)) == 0) {
            f_type = std::string(argv[i]).substr(strlen(F_OPT));
        } else if (strncmp(argv[i], I_OPT, strlen(I_OPT)) == 0) {
            input = std::string(argv[i]).substr(strlen(I_OPT));
        } else if (strncmp(argv[i], L_OPT, strlen(L_OPT)) == 0) {
            load_from = std::string(argv[i]).substr(strlen(L_OPT));
        } else {
            cerr << "Unknown option: " << argv[i] << endl;
            return -1;
        }
    }

    TypeRegistrator<TYPES>::register_all();
    SizeRegistrator<SIZES>::register_all();

    if (!load_from.empty()) {
        // try to load

        cerr << "Restoring saved simulation..." << endl;

        Param param;
        param.stop_flag = &stop_flag;
        Save save;
        if (!load_save(load_from, param, save)) {
            cerr << "Unable to restore simulation from provided filename." << endl;
            return -1;
        }

        bool dynamic = false;        
        if (SizeSet::sizes.count(save.size) == 0) {
            cerr << "No static field found for size " << save.size << ". Using dynamic." << endl;
            dynamic = true;
        }
        else {
            cerr << "Found static field of size " << save.size << "." << endl;
        }

        std::signal(SIGINT, sigint_handler);

        if (dynamic) {
            restore_dynamic_sim(load_from, param, p_type, v_type, f_type);
        }
        else {
            restore_static_sim(load_from, param, p_type, v_type, f_type, save.size);
        }
    }
    else {
        if (p_type.empty()) {
            cerr << "p-type option does not specified." << endl;
            return -1;
        }
        if (v_type.empty()) {
            cerr << "v-type option does not specified." << endl;
            return -1;
        }

        if (f_type.empty()) {
            cerr << "f-type option does not specified." << endl;
            return -1;
        }

        if (input.empty()) {
            cerr << "input option does not specified." << endl;
            return -1;
        }

        Param param;
        param.stop_flag = &stop_flag;
        if (!load_data(input, param)) {
            cerr << "Unable to load data from provided filename." << endl;
            return -1;
        }        

        bool dynamic = false;
        std::string size = std::to_string(param.n) + "x" + std::to_string(param.m);
        if (SizeSet::sizes.count(size) == 0) {
            cerr << "No static field found for size " << size << ". Using dynamic." << endl;
            dynamic = true;
        }
        else {
            cerr << "Found static field of size " << size << "." << endl;
        }

        std::signal(SIGINT, sigint_handler);

        if (dynamic) {
            run_dynamic_sim(param, p_type, v_type, f_type);
        }
        else {
            run_static_sim(param, p_type, v_type, f_type, size);
        }
    }          
    
    return 0;
}