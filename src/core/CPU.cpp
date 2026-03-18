#include "CPU.h"
#include "Config.h"
#include "Memory.h"
#include <stdexcept>
#include <cmath>

CPU::CPU(Memory& mem) :
    _memory{ mem },
    _accumulator{ 0 },
    _programCounter{ 0 },
    _instructionReg{ 0 },
    _stackPointer{ Config::INITIAL_STACK_POINTER },
    _indexReg{ 0 },
    _halted{ false },
    _waitingForInput{ false },
    _overflowFlag{ false },
    _cycles{ 0 },
    _outputBuffer{}
{
}

void CPU::reset()
{
    _accumulator = 0;
    _programCounter = 0;
    _instructionReg = 0;
    _stackPointer = Config::INITIAL_STACK_POINTER;
    _indexReg = 0;
    _halted = false;
    _waitingForInput = false;
    _overflowFlag = false;
    _cycles = 0;
    _outputBuffer.clear();
}

void CPU::provideInput(int value)
{
    setAccumulator(value);
    _waitingForInput = false;
}

void CPU::clearOutput()
{
    _outputBuffer.clear();
}

void CPU::setAccumulator(int value)
{
    if (std::abs(value) >= Config::OVERFLOW_LIMIT)
    {
        _overflowFlag = true;
        _accumulator = value % Config::OVERFLOW_LIMIT;
    }
    else
    {
        _overflowFlag = false;
        _accumulator = value;
    }
}

int CPU::readMemory(int address) const
{
    return _memory.read(address);
}

void CPU::writeMemory(int address, int value)
{
    _memory.write(address, value);
}

void CPU::step()
{
    if (_halted || _waitingForInput)
    {
        return;
    }

    _instructionReg = readMemory(_programCounter);
    _programCounter++;
    _cycles++;

    int rawOpcode = _instructionReg / 1000;
    int address = _instructionReg % 1000;
    Opcode opcode = static_cast<Opcode>(rawOpcode);

    switch (opcode)
    {
    case Opcode::HLT:
        if (address != 0)
        {
            throw std::runtime_error("Invalid operand for HLT");
        }
        _halted = true;
        break;
    case Opcode::NOP:
        if (address != 0)
        {
            throw std::runtime_error("Invalid operand for NOP");
        }
        break;
    case Opcode::LDA:
        setAccumulator(readMemory(address));
        break;
    case Opcode::STA:
        writeMemory(address, _accumulator);
        break;
    case Opcode::INP:
        if (address != 0)
        {
            throw std::runtime_error("Invalid operand for INP");
        }
        _waitingForInput = true;
        break;
    case Opcode::OUT:
        if (address != 0)
        {
            throw std::runtime_error("Invalid operand for OUT");
        }
        _outputBuffer.push_back(_accumulator);
        break;
    case Opcode::ADD:
        setAccumulator(_accumulator + readMemory(address));
        break;
    case Opcode::SUB:
        setAccumulator(_accumulator - readMemory(address));
        break;
    case Opcode::MUL:
        setAccumulator(_accumulator * readMemory(address));
        break;
    case Opcode::DIV:
        if (readMemory(address) == 0)
        {
            throw std::runtime_error("Division by zero");
        }
        setAccumulator(_accumulator / readMemory(address));
        break;
    case Opcode::MOD:
        if (readMemory(address) == 0)
        {
            throw std::runtime_error("Modulo by zero");
        }
        setAccumulator(_accumulator % readMemory(address));
        break;
    case Opcode::ADDI:
        setAccumulator(_accumulator + address);
        break;
    case Opcode::SUBI:
        setAccumulator(_accumulator - address);
        break;
    case Opcode::MULI:
        setAccumulator(_accumulator * address);
        break;
    case Opcode::CMA:
        if (address != 0)
        {
            throw std::runtime_error("Invalid operand for CMA");
        }
        setAccumulator(-_accumulator);
        break;
    case Opcode::INC:
        if (address != 0)
        {
            throw std::runtime_error("Invalid operand for INC");
        }
        setAccumulator(_accumulator + 1);
        break;
    case Opcode::DEC:
        if (address != 0)
        {
            throw std::runtime_error("Invalid operand for DEC");
        }
        setAccumulator(_accumulator - 1);
        break;
    case Opcode::BRA:
        _programCounter = address;
        break;
    case Opcode::BRZ:
        if (_accumulator == 0)
        {
            _programCounter = address;
        }
        break;
    case Opcode::BRP:
        if (_accumulator > 0)
        {
            _programCounter = address;
        }
        break;
    case Opcode::BRN:
        if (_accumulator < 0)
        {
            _programCounter = address;
        }
        break;
    case Opcode::BRO:
        if (_overflowFlag)
        {
            _programCounter = address;
        }
        break;
    case Opcode::PUSH:
        if (address != 0)
        {
            throw std::runtime_error("Invalid operand for PUSH");
        }
        writeMemory(_stackPointer, _accumulator);
        _stackPointer--;
        break;
    case Opcode::POP:
        if (address != 0)
        {
            throw std::runtime_error("Invalid operand for POP");
        }
        _stackPointer++;
        setAccumulator(readMemory(_stackPointer));
        break;
    case Opcode::CALL:
        writeMemory(_stackPointer, _programCounter);
        _stackPointer--;
        _programCounter = address;
        break;
    case Opcode::RET:
        if (address != 0)
        {
            throw std::runtime_error("Invalid operand for RET");
        }
        _stackPointer++;
        _programCounter = readMemory(_stackPointer);
        break;
    case Opcode::LDX:
        _indexReg = readMemory(address);
        break;
    case Opcode::STX:
        writeMemory(address, _indexReg);
        break;
    case Opcode::LDXI:
        _indexReg = address;
        break;
    case Opcode::INX:
        if (address != 0)
        {
            throw std::runtime_error("Invalid operand for INX");
        }
        _indexReg++;
        break;
    case Opcode::DEX:
        if (address != 0)
        {
            throw std::runtime_error("Invalid operand for DEX");
        }
        _indexReg--;
        break;
    case Opcode::LDAX:
        setAccumulator(readMemory(address + _indexReg));
        break;
    case Opcode::STAX:
        writeMemory(address + _indexReg, _accumulator);
        break;
    case Opcode::ADDX:
        setAccumulator(_accumulator + readMemory(address + _indexReg));
        break;
    case Opcode::SUBX:
        setAccumulator(_accumulator - readMemory(address + _indexReg));
        break;
    default:
        throw std::runtime_error("Unknown opcode");
    }
}

int CPU::getAccumulator() const
{
    return _accumulator;
}

int CPU::getProgramCounter() const
{
    return _programCounter;
}

int CPU::getInstructionRegister() const
{
    return _instructionReg;
}

int CPU::getStackPointer() const
{
    return _stackPointer;
}

int CPU::getIndexRegister() const
{
    return _indexReg;
}

const std::vector<int>& CPU::getOutputBuffer() const
{
    return _outputBuffer;
}

bool CPU::isHalted() const
{
    return _halted;
}

bool CPU::isWaitingForInput() const
{
    return _waitingForInput;
}

bool CPU::isOverflow() const
{
    return _overflowFlag;
}

int CPU::getCycles() const
{
    return _cycles;
}