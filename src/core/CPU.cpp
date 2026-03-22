#include "CPU.h"
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
    _statALU{ 0 },
    _statMemory{ 0 },
    _statControl{ 0 },
    _statIO{ 0 },
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
    _statALU = 0;
    _statMemory = 0;
    _statControl = 0;
    _statIO = 0;
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

void CPU::updateStats(Config::InstType type)
{
    switch (type)
    {
    case Config::InstType::ALU:
        _statALU++;
        break;
    case Config::InstType::MEMORY:
        _statMemory++;
        break;
    case Config::InstType::CONTROL:
        _statControl++;
        break;
    case Config::InstType::IO:
        _statIO++;
        break;
    }
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
    const Config::InstructionDef* def = Config::getInstructionDef(rawOpcode);
    if (def == nullptr)
    {
        throw std::runtime_error("Unknown opcode: " + std::to_string(rawOpcode));
	}
	updateStats(def->type);

    int address = _instructionReg % 1000;
    if (!(def->hasOperand) && address != 0)
    {
        throw std::runtime_error("Instruction " + std::string(def->name) + " has no operand" +
			" but got operand " + std::to_string(address));
    }

    Config::Opcode opcode = static_cast<Config::Opcode>(rawOpcode);
    switch (opcode)
    {
    case Config::Opcode::HLT:
        _halted = true;
        break;
    case Config::Opcode::NOP:
        break;
    case Config::Opcode::LDA:
        setAccumulator(readMemory(address));
        break;
    case Config::Opcode::STA:
        writeMemory(address, _accumulator);
        break;
    case Config::Opcode::INP:
        _waitingForInput = true;
        break;
    case Config::Opcode::OUT:
        _outputBuffer.push_back(_accumulator);
        break;
    case Config::Opcode::ADD:
        setAccumulator(_accumulator + readMemory(address));
        break;
    case Config::Opcode::SUB:
        setAccumulator(_accumulator - readMemory(address));
        break;
    case Config::Opcode::MUL:
        setAccumulator(_accumulator * readMemory(address));
        break;
    case Config::Opcode::DIV:
        if (readMemory(address) == 0)
        {
            throw std::runtime_error("Division by zero");
        }
        setAccumulator(_accumulator / readMemory(address));
        break;
    case Config::Opcode::MOD:
        if (readMemory(address) == 0)
        {
            throw std::runtime_error("Modulo by zero");
        }
        setAccumulator(_accumulator % readMemory(address));
        break;
    case Config::Opcode::ADDI:
        setAccumulator(_accumulator + address);
        break;
    case Config::Opcode::SUBI:
        setAccumulator(_accumulator - address);
        break;
    case Config::Opcode::MULI:
        setAccumulator(_accumulator * address);
        break;
	case Config::Opcode::DIVI:
        if (address == 0)
        {
            throw std::runtime_error("Division by zero");
        }
        setAccumulator(_accumulator / address);
		break;
    case Config::Opcode::MODI:
        if (address == 0)
        {
            throw std::runtime_error("Modulo by zero");
        }
        setAccumulator(_accumulator % address);
		break;
    case Config::Opcode::CMA:
        setAccumulator(-_accumulator);
        break;
    case Config::Opcode::INC:
        setAccumulator(_accumulator + 1);
        break;
    case Config::Opcode::DEC:
        setAccumulator(_accumulator - 1);
        break;
    case Config::Opcode::BRA:
        _programCounter = address;
        break;
    case Config::Opcode::BRZ:
        if (_accumulator == 0)
        {
            _programCounter = address;
        }
        break;
    case Config::Opcode::BRP:
        if (_accumulator > 0)
        {
            _programCounter = address;
        }
        break;
    case Config::Opcode::BRN:
        if (_accumulator < 0)
        {
            _programCounter = address;
        }
        break;
    case Config::Opcode::BRO:
        if (_overflowFlag)
        {
            _programCounter = address;
        }
        break;
    case Config::Opcode::PUSH:
        writeMemory(_stackPointer, _accumulator);
        _stackPointer--;
        break;
    case Config::Opcode::POP:
        _stackPointer++;
        setAccumulator(readMemory(_stackPointer));
        break;
    case Config::Opcode::CALL:
        writeMemory(_stackPointer, _programCounter);
        _stackPointer--;
        _programCounter = address;
        break;
    case Config::Opcode::RET:
        _stackPointer++;
        _programCounter = readMemory(_stackPointer);
        break;
    case Config::Opcode::LDX:
        _indexReg = readMemory(address);
        break;
    case Config::Opcode::STX:
        writeMemory(address, _indexReg);
        break;
    case Config::Opcode::LDXI:
        _indexReg = address;
        break;
    case Config::Opcode::INX:
        _indexReg++;
        break;
    case Config::Opcode::DEX:
        _indexReg--;
        break;
    case Config::Opcode::LDAX:
        setAccumulator(readMemory(address + _indexReg));
        break;
    case Config::Opcode::STAX:
        writeMemory(address + _indexReg, _accumulator);
        break;
    case Config::Opcode::ADDX:
        setAccumulator(_accumulator + readMemory(address + _indexReg));
        break;
    case Config::Opcode::SUBX:
        setAccumulator(_accumulator - readMemory(address + _indexReg));
        break;
	case Config::Opcode::MULX:
        setAccumulator(_accumulator * readMemory(address + _indexReg));
		break;
    case Config::Opcode::DIVX:
    {
        int divisor = readMemory(address + _indexReg);
        if (divisor == 0)
        {
            throw std::runtime_error("Division by zero");
        }
        setAccumulator(_accumulator / divisor);
        break;
    }
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