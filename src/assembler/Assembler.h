#pragma once
#include <string>
#include <vector>
#include <unordered_map>

struct AssemblerResult
{
    bool success{ false };
    std::string errorMessage{ "" };
    std::vector<int> machineCode{};
    std::unordered_map<int, std::string> reverseSymbolTable{};
};

struct ParsedInstruction
{
    int originalLine;
    std::string mnemonic;
    std::string operand;
};

class Assembler
{
public:
    Assembler();

    AssemblerResult compile(const std::string& sourceCode);

private:
    std::unordered_map<std::string, int> _symbolTable;
    std::vector<int> _machineCode;
    std::vector<ParsedInstruction> _parsedInstructions;
    std::string _errorMessage;

    void reset();
    bool runPassOne(const std::string& sourceCode);
    bool runPassTwo();

    std::string trim(const std::string& str) const;
    std::vector<std::string> split(const std::string& str) const;
    std::string removeComments(const std::string& str) const;
    std::string toUpper(const std::string& str) const;
    bool isNumber(const std::string& str) const;
    bool isValidIdentifier(const std::string& str) const;
};