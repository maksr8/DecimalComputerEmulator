#pragma once
#include <vector>
#include "Config.h"

class Memory;

class CPU
{
public:
    explicit CPU(Memory& mem);

    void reset();
    void step();
    void provideInput(int value);
    void clearOutput();

    int getAccumulator() const;
    int getProgramCounter() const;
    int getInstructionRegister() const;
    int getStackPointer() const;
    int getIndexRegister() const;
    const std::vector<int>& getOutputBuffer() const;

    bool isHalted() const;
    bool isWaitingForInput() const;
    bool isOverflow() const;
    int getCycles() const;

private:
    Memory& _memory;

    int _accumulator;
    int _programCounter;
    int _instructionReg;
    int _stackPointer;
    int _indexReg;

    bool _halted;
    bool _waitingForInput;
    bool _overflowFlag;

    int _cycles;
    int _statALU;
    int _statMemory;
    int _statControl;
    int _statIO;

    std::vector<int> _outputBuffer;

    void setAccumulator(int value);
    int readMemory(int address) const;
    void writeMemory(int address, int value);
    void updateStats(Config::InstType type);
};