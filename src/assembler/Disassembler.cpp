#include "Disassembler.h"
#include "../core/Config.h"

std::string Disassembler::decode(int machineWord)
{
    int opcode = machineWord / 1000;
    int operand = machineWord % 1000;

	const Config::InstructionDef* def = Config::getInstructionDef(opcode);

	if (def != nullptr)
    {
        std::string mnemonic{ def->name };

        if (def->hasOperand)
        {
            std::string result;
            result.reserve(mnemonic.size() + 1 + 11); // use 11 to be sure any int can fit
            result += mnemonic;
            result += ' ';
            result += std::to_string(operand);
            return result;
        }
        else
        {
            if (operand != 0)
            {
                std::string result;
                result.reserve(Config::DAT_MNEMONIC.size() + 1 + 11);
                result.append(Config::DAT_MNEMONIC.data(), Config::DAT_MNEMONIC.size());
                result += ' ';
                result += std::to_string(machineWord);
                return result;
            }
            return mnemonic;
        }
    }

    std::string result;
    result.reserve(Config::DAT_MNEMONIC.size() + 1 + 11);
    result.append(Config::DAT_MNEMONIC.data(), Config::DAT_MNEMONIC.size());
    result += ' ';
    result += std::to_string(machineWord);
    return result;
}