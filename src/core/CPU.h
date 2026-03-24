#pragma once
#include <vector>
#include "Config.h"

class Memory;

struct StepResult {
    bool memoryWasWritten{ false };
    int writtenAddress{ -1 };
};

class CPU
{
public:
    explicit CPU(Memory& mem);

    void reset();
    StepResult step();
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

    int getStatALU() const;
    int getStatMemory() const;
    int getStatControl() const;
    int getStatIO() const;

private:
    Memory& _memory;

    int _accumulator{ 0 };
    int _programCounter{ 0 };
    int _instructionReg{ 0 };
    int _stackPointer{ Config::INITIAL_STACK_POINTER };
    int _indexReg{ 0 };

    bool _halted{ false };
    bool _waitingForInput{ false };
    bool _overflowFlag{ false };

    int _cycles{ 0 };
    int _statALU{ 0 };
    int _statMemory{ 0 };
    int _statControl{ 0 };
    int _statIO{ 0 };

    std::vector<int> _outputBuffer{};

    const Config::InstructionDef* _currentInstructionDef{ nullptr };
    int _lastWrittenAddress{ -1 };

    void setAccumulator(int value);
    void setIndexRegister(int value);
    int readMemory(int address) const;
    void writeMemory(int address, int value);
    void updateStats(Config::InstType type);
};