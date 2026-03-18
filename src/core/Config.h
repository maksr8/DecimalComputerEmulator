#pragma once

namespace Config
{
    constexpr int MEMORY_SIZE{ 1000 };
    constexpr int OVERFLOW_LIMIT{ 100000 };
    constexpr int INITIAL_STACK_POINTER{ 999 };
}

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
    SUBX = 68
};