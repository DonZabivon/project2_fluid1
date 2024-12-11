# Большое домашнее задание 2

## Fluid Simulator part 1

### Описание реализации

1. Код был вынесен из глобального неймспейса в класс FluidSimulator (файл `sim.h`).

2. Класс был сделан шаблоном:

```C++
template<typename PType, typename VType, typename FType, size_t N, size_t M, template<typename PType1, typename VType1, typename FType1, size_t N1, size_t M1> typename Storage>
class FluidSimulator;
```

Последний параметр шаблона, `Storage`, сам является шаблонам класса с точно такими же параметрами. Этот класс предназначен для хранения двумерных и трехмерных массивов, зависимых от размера поля (`field`, `p`, `old_p`, `velocity`, `velocity_flow`, `last_use` и `dirs`). Целью такого разделения является сделать основной класс `FluidSimulator` независимым от стратегии работы с памятью. Следующая структура хранит все данные симулятора (файл `data.h`):

```C++
template<typename PType, typename VType, typename FType, size_t N, size_t M>
struct SimData {
    char field[N][M + 1]{};
    PType p[N][M]{};
    PType old_p[N][M]{};
    VType velocity[N][M][4]{};
    FType velocity_flow[N][M][4]{};
    int last_use[N][M]{};
    int dirs[N][M]{};
 ```

 Это хранилище со статическими массивами. Альтернатива с динамическими массивами специализируется при N = M = 0, т.е., нулевые размеры определяют симулятор, использующий динамическое выделение памяти.

 ```C++
template<typename PType, typename VType, typename FType>
struct SimData<PType, VType, FType, 0, 0> {
    size_t H{}; // Вместо N, которое 0. Будет задано позже.
    size_t W{}; // Вместо M, которое 0. Будет задано позже.
    vector<vector<char>> field{};
    vector<vector<PType>> p{};
    vector<vector<PType>> old_p{};
    vector<vector<vector<VType>>> velocity{};
    vector<vector<vector<FType>>> velocity_flow{};
    vector<vector<int>> last_use{};
    vector<vector<int>> dirs{};
 ```

 3. Был написан шаблон Fixed<N, K> (файл `fixed.h`):

```C++
template<typename T, uint8_t N, uint8_t K, bool Fast=false> 
struct FixedT;
 ```

 Он предназначен для хранения как типа `FIXED`, так и `FAST_FIXED`. Последний задается частичными специализациями,
 сначала 4 специализации для 4 базовах типов:

```C++
template <uint8_t N, uint8_t K>
using FastFixed8T = FixedT<int_fast8_t, N, K, true>;

template <uint8_t N, uint8_t K>
using FastFixed16T = FixedT<int_fast16_t, N, K, true>;

template <uint8_t N, uint8_t K>
using FastFixed32T = FixedT<int_fast32_t, N, K, true>;

template <uint8_t N, uint8_t K>
using FastFixed64T = FixedT<int_fast64_t, N, K, true>;
 ```

 А затем более конкретные специализации для каждого из 64 возможных `N`:

```C++
template <uint8_t K>
using FastFixedN1 = FastFixed8T<1, K>;
//...
template <uint8_t K>
using FastFixedN9 = FastFixed16T<9, K>;
//...
 ```

 Для преобразования различных FIXED-типов был разработан шаблонный конструктор:

```C++
template<typename T, uint8_t N, uint8_t K, bool Fast=false> 
struct FixedT {
    template<typename T1, uint8_t N1, uint8_t K1, bool Fast1>
    constexpr FixedT(const FixedT<T1, N1, K1, Fast1>& rhs);

 ```

 4. С помощью макросов опции компиляции преобразуются в шаблонные типы. Например, список `FLOAT,FAST_FIXED(13,7),FIXED(32,5),DOUBLE`
преобразуется макросами в `float, FastFixedN13<7>, FixedT<int32_t, 32, 5>, double`. Затем этот список передается в шаблон (`TypeRegistrator`, файл `type_set.h`), который регистрирует все перечисленные типы. Есть `unordered_map`, который связывает имя типа с его значением по умолчанию. Для того, чтобы можно было хранить в одном контейнере значения разных типов, используется `std::variant`: 

```C++
struct TypeSet {
    static std::unordered_map<std::string, std::variant<TYPES>> types;

    template<typename T>
    static void register_one(T arg) {
        types.insert({ name<T>(), std::variant<TYPES>(arg) });
    }
};
```

Список типов для `std::variant` берется прямо из описания `TYPES`, которое преобразуется макросами в корректную последовательность типов. Шаблонная функция `register_one` принимает аргумент определенного типа, создает экземпляр `std::variant` и помещает его в контейнер. Позже это будет использоваться для получения типа из его имени. Шаблонная функция `name<T>` возвращает имя типа (не системное, а принятое в программе для различения типов):

```C++
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
```

Неспециализированный шаблон возвращает имя, полученное из функции `FixedT::name()`.

У класса `TypeSet` есть наследники, по одному для разных размеров списка типов:

```C++
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
...
```

Реализовано семь таких структур, то есть, поддерживается размер списка типов не более семи типов. Эти структуры используются следующим образом:

```C++
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
```

Т.е., для определенного размера списка вызывается метод из соответствующей структуры. После запуска программы типы регистрируются следующей конструкцией:

```C++
TypeRegistrator<TYPES>::register_all();
```

Аналогичная стратегия используется и для размеров (файл `size_set.h`). Для хранения размера используется простая шаблонная структура:

```C++
template <size_t N, size_t M>
struct Size {
    static constexpr size_t n = N;
    static constexpr size_t m = M;
    static constexpr std::string name() {
        return std::to_string(N) + "x" + std::to_string(M);
    }
};
```

Точно так же используется `std::variant` для хранения любого из возможных размеров:

```C++
struct SizeSet {
    static std::unordered_map<std::string, std::variant<SIZES>> sizes;

    template<typename T>
    static void register_one(T arg) {        
        sizes.insert({ T::name(), std::variant<SIZES>(arg) });
    }
};
```

5. Для преобразования параметров из рантайма в статические наборы параметров для создания экземпляра класса симулятора используется следующий подход.
Сначала считывается входной файл и определяется, есть ли скомпилированные экземпляры с заданным размером, то есть, будет ли использоваться класс со статическим или динамическим распределением памяти. Далее, на основе опций из командной строки, задающей используемые типы, из реестра типов извлекаются экземпляры `std::variant`, содержащие значения заданных типов. Статические параметры типов извлекаются с помощью метода `std::variant::visit`:

```C++
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
```

Если размер поля из входного файла найден в реестре, то тогда дополнительно используется четвертый вложенный `std::variant::visit` для извлечения размера.

Класс `SimRunner` определен следующим образом:

```C++
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
```

Следует сказать, что использование предкомпилированных экземпляров со статическим размером полей вызывает некоторые проблемы. Компилятор возмущается и предлагает задуматься о размещении данных в куче, а не на стеке. Может возникнуть переполнение стека при запуске программы, иногда может понадобится увеличить размер стека с помощью опций компилятора. Компиляция выполняется очень долго и потребляет очень много оперативной памяти, что также зависит и от размера списка типов. Чем больше типов, тем больше альтернатив (третья степень от количества типов, согласно правилам комбинаторики). Для 10 типов получается 1000 альтернатив, умноженное на размер списка размеров. И все это должно скомпилироваться.

6. Реализовано считывание данных из файла. Файл имеет очень простую структуру:

```
14 5 0.1 0.01 1000
#####
#.  #
#.# #
#.# #
#.# #
#.# #
#.# #
#.# #
#...#
#####
#   #
#   #
#   #
#####
```

На первой строке идут размеры поля, затем константа g, и затем значения плотности для воздуха и жидкости. На следующих строках размещается поле.

Реализовано сохранение и загрузка симуляции. Сохранение выполняется при нажатии на комбинацию `CTRL-C`. Программа сохраняет в бинарный файл `fluid.save` все данные симуляции - параметры экземпляра (типы, размер поля - для того, чтобы использовать точно такой же экземпляр симулятора) и значения всех массивов, и завершает работу (текущий тик полностью завершается, прежде чем это произойдет). Загрузка сохранения выполняется при передаче праметра `--load=<имя файла сохранения>` при запуске программы, при этом остальные опции будут проигнорированны.

Для компиляции программы можно использовать команду `make`. В проекте только один файл .cpp, код в основном распределен по заголовочным файлам, так что с компиляцией не должно возникнуть трудностей. Компилятор должен поддерживать C++20.
 

