#pragma once
#include <vector>

class Memory {
public:
    Memory();

    int read(int address) const;
    void write(int address, int value);
    void load(const std::vector<int>& program);
    void reset();

private:
    std::vector<int> _data;
};