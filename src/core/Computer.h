#pragma once
#include "CPU.h"
#include "Memory.h"
#include <vector>

class Computer
{
public:
    Computer();

    void loadProgram(const std::vector<int>& program);
    void reset();
    StepResult step();
    void provideInput(int value);

    const CPU& getCPU() const;
    const Memory& getMemory() const;

private:
    Memory _memory;
    CPU _cpu;
};