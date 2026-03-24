#include "Computer.h"
#include "Config.h"
#include <stdexcept>

Computer::Computer() :
    _memory{},
    _cpu{ _memory }
{
}

void Computer::loadProgram(const std::vector<int>& program)
{
    int programSize = static_cast<int>(program.size());
    if (programSize > Config::MEMORY_SIZE)
    {
        throw std::out_of_range("Program too large for memory");
    }
    reset();
    _memory.load(program);
}

void Computer::reset()
{
    _memory.reset();
    _cpu.reset();
}

StepResult Computer::step()
{
    return _cpu.step();
}

void Computer::provideInput(int value)
{
    _cpu.provideInput(value);
}

const CPU& Computer::getCPU() const
{
    return _cpu;
}

const Memory& Computer::getMemory() const
{
    return _memory;
}