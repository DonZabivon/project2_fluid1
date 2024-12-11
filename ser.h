#pragma once

#include <string>
#include <iostream>

template<typename T>
void write(ostream& out, const T& val) {
    out.write((const char*)&val.v, sizeof(val.v));
}

template<>
void write<float>(ostream& out, const float& val) {
    out.write((const char*)&val, sizeof(val));
}

template<>
void write<double>(ostream& out, const double& val) {
    out.write((const char*)&val, sizeof(val));
}

template<>
void write<int>(ostream& out, const int& val) {
    out.write((const char*)&val, sizeof(val));
}

template<>
void write<char>(ostream& out, const char& val) {
    out.write((const char*)&val, sizeof(val));
}

template<typename T>
T read(istream& in) {
    T val;
    in.read((char*)&val.v, sizeof(val.v));
    return val;
}

template<>
float read<float>(istream& in) {
    float val;
    in.read((char*)&val, sizeof(val));
    return val;
}

template<>
double read<double>(istream& in) {
    double val;
    in.read((char*)&val, sizeof(val));
    return val;
}

template<>
int read<int>(istream& in) {
    int val;
    in.read((char*)&val, sizeof(val));
    return val;
}

template<>
char read<char>(istream& in) {
    char val;
    in.read((char*)&val, sizeof(val));
    return val;
}
