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
            return mnemonic + " " + std::to_string(operand);
        }
        else
        {
            if (operand != 0)
            {
                return "DAT " + std::to_string(machineWord);
            }
            return mnemonic;
        }
    }

    return "DAT " + std::to_string(machineWord);
}