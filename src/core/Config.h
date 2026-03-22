#pragma once
#include <string>
#include <unordered_map>
#include <optional>

namespace Config
{
    constexpr int MEMORY_SIZE{ 1000 };
    constexpr int MAX_PROGRAM_SIZE{ 900 };
    constexpr int OVERFLOW_LIMIT{ 100000 };
    constexpr int INITIAL_STACK_POINTER{ 999 };

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

    enum class InstType {
        ALU,
        MEMORY,
        CONTROL,
        IO,
        NONE
    };

    struct InstructionDef
    {
        std::string_view name;
        Opcode opcode;
        bool hasOperand;
        InstType type;
    };

    inline constexpr InstructionDef INSTRUCTIONS[] =
    {
        {"HLT", Opcode::HLT, false, InstType::CONTROL},
        {"NOP", Opcode::NOP, false, InstType::CONTROL},
        {"LDA", Opcode::LDA, true,  InstType::MEMORY},
        {"STA", Opcode::STA, true,  InstType::MEMORY},
        {"INP", Opcode::INP, false, InstType::IO},
        {"OUT", Opcode::OUT, false, InstType::IO},
        {"ADD", Opcode::ADD, true,  InstType::ALU},
        {"SUB", Opcode::SUB, true,  InstType::ALU},
        {"MUL", Opcode::MUL, true,  InstType::ALU},
        {"DIV", Opcode::DIV, true,  InstType::ALU},
        {"MOD", Opcode::MOD, true,  InstType::ALU},
        {"ADDI", Opcode::ADDI, true, InstType::ALU},
        {"SUBI", Opcode::SUBI, true, InstType::ALU},
        {"MULI", Opcode::MULI, true, InstType::ALU},
        {"DIVI", Opcode::DIVI, true, InstType::ALU},
        {"MODI", Opcode::MODI, true, InstType::ALU},
        {"CMA", Opcode::CMA, false, InstType::ALU},
        {"INC", Opcode::INC, false, InstType::ALU},
        {"DEC", Opcode::DEC, false, InstType::ALU},
        {"BRA", Opcode::BRA, true,  InstType::CONTROL},
        {"BRZ", Opcode::BRZ, true,  InstType::CONTROL},
        {"BRP", Opcode::BRP, true,  InstType::CONTROL},
        {"BRN", Opcode::BRN, true,  InstType::CONTROL},
        {"BRO", Opcode::BRO, true,  InstType::CONTROL},
        {"PUSH", Opcode::PUSH, false, InstType::MEMORY},
        {"POP", Opcode::POP, false, InstType::MEMORY},
        {"CALL", Opcode::CALL, true, InstType::CONTROL},
        {"RET", Opcode::RET, false, InstType::CONTROL},
        {"LDX", Opcode::LDX, true,  InstType::MEMORY},
        {"STX", Opcode::STX, true,  InstType::MEMORY},
        {"LDXI", Opcode::LDXI, true, InstType::ALU},
        {"INX", Opcode::INX, false, InstType::ALU},
        {"DEX", Opcode::DEX, false, InstType::ALU},
        {"LDAX", Opcode::LDAX, true, InstType::MEMORY},
        {"STAX", Opcode::STAX, true, InstType::MEMORY},
        {"ADDX", Opcode::ADDX, true, InstType::ALU},
        {"SUBX", Opcode::SUBX, true, InstType::ALU},
        {"MULX", Opcode::MULX, true, InstType::ALU},
        {"DIVX", Opcode::DIVX, true, InstType::ALU}
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

    //to get both name and hasOperand
    inline const InstructionDef* getInstructionDef(int opcodeValue)
    {
        for (const auto& def : INSTRUCTIONS)
        {
            if (static_cast<int>(def.opcode) == opcodeValue) return &def;
        }
        return nullptr;
    }

    //to get both opcode and hasOperand
    inline const InstructionDef* getInstructionDef(std::string_view name)
    {
        for (const auto& def : INSTRUCTIONS)
        {
            if (def.name == name) return &def;
        }
        return nullptr;
    }
}
