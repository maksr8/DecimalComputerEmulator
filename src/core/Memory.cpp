#include "Memory.h"
#include "Config.h"
#include <stdexcept>
#include <algorithm>

Memory::Memory() :
    _data{ std::vector<int>(Config::MEMORY_SIZE, 0) }
{
}

int Memory::read(int address) const
{
    if (address < 0 || address >= Config::MEMORY_SIZE)
    {
        throw std::out_of_range("Memory access violation");
    }
    return _data[address];
}

void Memory::write(int address, int value)
{
    if (address < 0 || address >= Config::MEMORY_SIZE)
    {
        throw std::out_of_range("Memory access violation");
    }
    _data[address] = value % Config::OVERFLOW_LIMIT;
}

void Memory::load(const std::vector<int>& program)
{
    std::transform(program.begin(), program.end(), _data.begin(),
        [](int val)
        {
            return val % Config::OVERFLOW_LIMIT;
        }
    );
}

void Memory::reset()
{
    std::fill(_data.begin(), _data.end(), 0);
}