#include "Assembler.h"
#include "../core/Config.h"
#include <sstream>
#include <cctype>

Assembler::Assembler() :
    _symbolTable{},
    _machineCode{},
    _parsedInstructions{},
    _errorMessage{ "" }
{
}

void Assembler::reset()
{
    _symbolTable.clear();
    _machineCode.assign(Config::MEMORY_SIZE, 0);
    _parsedInstructions.clear();
    _errorMessage = "";
}

AssemblerResult Assembler::compile(const std::string& sourceCode)
{
    reset();
    AssemblerResult result;

    if (!runPassOne(sourceCode) || !runPassTwo())
    {
        result.success = false;
        result.errorMessage = _errorMessage;
        return result;
    }

    result.success = true;
    result.machineCode = _machineCode;

    for (const auto& pair : _symbolTable)
    {
        result.reverseSymbolTable[pair.second] = pair.first;
    }

    return result;
}

bool Assembler::runPassOne(const std::string& sourceCode)
{
    std::stringstream stream{ sourceCode };
    std::string line;
    int currentAddress{ 0 };
    int lineNumber{ 0 };

    while (std::getline(stream, line))
    {
        lineNumber++;
        line = removeComments(line);
        line = trim(line);

        if (line.empty())
        {
            continue;
        }

        std::vector<std::string> tokens = split(line);

        if (tokens.empty())
        {
            continue;
        }

        if (tokens[0].back() == ':')
        {
            std::string label = tokens[0].substr(0, tokens[0].size() - 1);

            if (!isValidIdentifier(label))
            {
                _errorMessage = "Invalid label format '" + label + "' at line " + std::to_string(lineNumber);
                return false;
            }

            if (_symbolTable.find(label) != _symbolTable.end())
            {
                _errorMessage = "Duplicate label '" + label + "' at line " + std::to_string(lineNumber);
                return false;
            }

            _symbolTable[label] = currentAddress;
            tokens.erase(tokens.begin());
        }

        if (!tokens.empty())
        {
            if (currentAddress >= Config::MAX_PROGRAM_SIZE)
            {
                _errorMessage = "Program exceeds memory size limit (Max " + std::to_string(Config::MAX_PROGRAM_SIZE) + ")";
                return false;
            }

            if (tokens.size() > 2)
            {
                _errorMessage = "Too many arguments at line " + std::to_string(lineNumber);
                return false;
            }

            ParsedInstruction instruction;
            instruction.originalLine = lineNumber;
            instruction.mnemonic = toUpper(tokens[0]);
            instruction.operand = (tokens.size() == 2) ? tokens[1] : "";

            _parsedInstructions.push_back(instruction);
            currentAddress++;
        }
    }

    return true;
}

bool Assembler::runPassTwo()
{
    for (int i = 0; i < static_cast<int>(_parsedInstructions.size()); i++)
    {
        const ParsedInstruction& instruction = _parsedInstructions[i];

        if (instruction.mnemonic == "DAT")
        {
            int value{ 0 };
            if (!instruction.operand.empty())
            {
                if (isNumber(instruction.operand))
                {
                    value = std::stoi(instruction.operand);
                }
                else
                {
                    if (_symbolTable.find(instruction.operand) == _symbolTable.end())
                    {
                        _errorMessage = "Undefined label '" + instruction.operand + "' at line " + std::to_string(instruction.originalLine);
                        return false;
                    }
                    value = _symbolTable.at(instruction.operand);
                }

                if (std::abs(value) >= Config::OVERFLOW_LIMIT)
                {
                    _errorMessage = "DAT value '" + std::to_string(value) + "' exceeds machine limits (+-" + std::to_string(Config::OVERFLOW_LIMIT - 1) + ") at line " + std::to_string(instruction.originalLine);
                    return false;
                }
            }
            _machineCode[i] = value;
            continue;
        }

        const Config::InstructionDef* def = Config::getInstructionDef(instruction.mnemonic);

        if (def == nullptr)
        {
            _errorMessage = "Unknown instruction '" + instruction.mnemonic + "' at line " + std::to_string(instruction.originalLine);
            return false;
        }

        if (!def->hasOperand && !instruction.operand.empty())
        {
            _errorMessage = "Instruction '" + instruction.mnemonic + "' does not take operand at line " + std::to_string(instruction.originalLine);
            return false;
        }

        if (def->hasOperand && instruction.operand.empty())
        {
            _errorMessage = "Instruction '" + instruction.mnemonic + "' requires an operand at line " + std::to_string(instruction.originalLine);
            return false;
        }

        int opcode = static_cast<int>(def->opcode);
        int operand{ 0 };

        if (!instruction.operand.empty())
        {
            if (isNumber(instruction.operand))
            {
                operand = std::stoi(instruction.operand);
            }
            else
            {
                if (_symbolTable.find(instruction.operand) == _symbolTable.end())
                {
                    _errorMessage = "Undefined label '" + instruction.operand + "' at line " + std::to_string(instruction.originalLine);
                    return false;
                }
                operand = _symbolTable.at(instruction.operand);
            }

            if (operand < 0 || operand >= Config::MEMORY_SIZE)
            {
                _errorMessage = "Operand '" + std::to_string(operand) + "' is out of memory bounds (0-" + std::to_string(Config::MEMORY_SIZE - 1) + ") at line " + std::to_string(instruction.originalLine);
                return false;
            }
        }

        _machineCode[i] = (opcode * 1000) + operand;
    }

    return true;
}

std::string Assembler::trim(const std::string& str) const
{
    int start{ 0 };
    int end{ static_cast<int>(str.length()) - 1 };

    while (start <= end && std::isspace(str[start]))
    {
        start++;
    }
    while (end >= start && std::isspace(str[end]))
    {
        end--;
    }

    return str.substr(start, end - start + 1);
}

std::vector<std::string> Assembler::split(const std::string& str) const
{
    std::vector<std::string> tokens;
    std::stringstream stream{ str };
    std::string token;

    while (stream >> token)
    {
        tokens.push_back(token);
    }

    return tokens;
}

std::string Assembler::removeComments(const std::string& str) const
{
    return str.substr(0, str.find(';'));
}

std::string Assembler::toUpper(const std::string& str) const
{
    std::string upperStr = str;
    for (int i = 0; i < static_cast<int>(upperStr.length()); i++)
    {
        upperStr[i] = static_cast<char>(std::toupper(upperStr[i]));
    }
    return upperStr;
}

bool Assembler::isNumber(const std::string& str) const
{
    if (str.empty())
    {
        return false;
    }

    int start = (str[0] == '-') ? 1 : 0;

    if (start == 1 && str.length() == 1)
    {
        return false;
    }

    for (int i = start; i < static_cast<int>(str.length()); i++)
    {
        if (!std::isdigit(str[i]))
        {
            return false;
        }
    }

    return true;
}

bool Assembler::isValidIdentifier(const std::string& str) const
{
    if (str.empty())
    {
        return false;
    }

    if (!std::isalpha(str[0]) && str[0] != '_')
    {
        return false;
    }

    for (char c : str)
    {
        if (!std::isalnum(c) && c != '_')
        {
            return false;
        }
    }

    return true;
}