#pragma once
#include <string>
#include <unordered_map>
#include <optional>

enum class Opcode : int
{
    HLT = 0,
    NOP = 1,
    LDA = 10,
    STA = 11,
    INP = 18,
    OUT = 19,
    ADD = 20,
    SUB = 21,
    MUL = 22,
    DIV = 23,
    MOD = 24,
    ADDI = 30,
    SUBI = 31,
    MULI = 32,
    DIVI = 33,
    MODI = 34,
    CMA = 37,
    INC = 38,
    DEC = 39,
    BRA = 40,
    BRZ = 41,
    BRP = 42,
    BRN = 43,
    BRO = 44,
    PUSH = 50,
    POP = 51,
    CALL = 52,
    RET = 53,
    LDX = 60,
    STX = 61,
    LDXI = 62,
    INX = 63,
    DEX = 64,
    LDAX = 65,
    STAX = 66,
    ADDX = 67,
    SUBX = 68,
    MULX = 69,
    DIVX = 70,
};

namespace Config
{
    constexpr int MEMORY_SIZE{ 1000 };
    constexpr int MAX_PROGRAM_SIZE{ 900 };
    constexpr int OVERFLOW_LIMIT{ 100000 };
    constexpr int INITIAL_STACK_POINTER{ 999 };

    struct InstructionDef
    {
        std::string_view name;
        Opcode opcode;
        bool hasOperand;
    };

    inline constexpr InstructionDef INSTRUCTIONS[] =
    {
        {"HLT", Opcode::HLT, false},
        {"NOP", Opcode::NOP, false},
        {"LDA", Opcode::LDA, true},
        {"STA", Opcode::STA, true},
        {"INP", Opcode::INP, false},
        {"OUT", Opcode::OUT, false},
        {"ADD", Opcode::ADD, true},
        {"SUB", Opcode::SUB, true},
        {"MUL", Opcode::MUL, true},
        {"DIV", Opcode::DIV, true},
        {"MOD", Opcode::MOD, true},
        {"ADDI", Opcode::ADDI, true},
        {"SUBI", Opcode::SUBI, true},
        {"MULI", Opcode::MULI, true},
        {"DIVI", Opcode::DIVI, true},
        {"MODI", Opcode::MODI, true},
        {"CMA", Opcode::CMA, false},
        {"INC", Opcode::INC, false},
        {"DEC", Opcode::DEC, false},
        {"BRA", Opcode::BRA, true},
        {"BRZ", Opcode::BRZ, true},
        {"BRP", Opcode::BRP, true},
        {"BRN", Opcode::BRN, true},
        {"BRO", Opcode::BRO, true},
        {"PUSH", Opcode::PUSH, false},
        {"POP", Opcode::POP, false},
        {"CALL", Opcode::CALL, true},
        {"RET", Opcode::RET, false},
        {"LDX", Opcode::LDX, true},
        {"STX", Opcode::STX, true},
        {"LDXI", Opcode::LDXI, true},
        {"INX", Opcode::INX, false},
        {"DEX", Opcode::DEX, false},
        {"LDAX", Opcode::LDAX, true},
        {"STAX", Opcode::STAX, true},
        {"ADDX", Opcode::ADDX, true},
        {"SUBX", Opcode::SUBX, true},
        {"MULX", Opcode::MULX, true},
        {"DIVX", Opcode::DIVX, true}
    };

    inline std::optional<std::string_view> getOpcodeName(int opcodeValue)
    {
        for (const auto& def : INSTRUCTIONS)
        {
            if (static_cast<int>(def.opcode) == opcodeValue)
            {
                return def.name;
            }
        }
        return std::nullopt;
    }

    inline std::optional<int> getOpcodeFromString(std::string_view name)
    {
        for (const auto& def : INSTRUCTIONS)
        {
            if (def.name == name)
            {
                return static_cast<int>(def.opcode);
            }
        }
        return std::nullopt;
    }

	//to get both name and hasOperand
    inline const InstructionDef* getInstructionDef(int opcodeValue)
    {
        for (const auto& def : INSTRUCTIONS)
        {
            if (static_cast<int>(def.opcode) == opcodeValue) return &def;
        }
        return nullptr;
    }

    inline const InstructionDef* getInstructionDef(std::string_view name)
    {
        for (const auto& def : INSTRUCTIONS)
        {
            if (def.name == name) return &def;
        }
        return nullptr;
    }

  //  inline std::optional<bool> instructionRequiresOperand(int opcodeValue)
  //  {
  //      for (const auto& def : INSTRUCTIONS)
  //      {
  //          if (static_cast<int>(def.opcode) == opcodeValue)
  //          {
  //              return def.hasOperand;
  //          }
  //      }
		//return std::nullopt;
  //  }
}
