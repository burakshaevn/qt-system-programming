#include "structures/codeline.h"

CodeLine::CodeLine()
    : label_(""), command_(""), firstOperand_(""), secondOperand_("")
{
}

CodeLine::CodeLine(const std::string& command)
    : label_(""), command_(command), firstOperand_(""), secondOperand_("")
{
}

CodeLine::CodeLine(const std::string& command, const std::string& firstOperand)
    : label_(""), command_(command), firstOperand_(firstOperand), secondOperand_("")
{
}

CodeLine::CodeLine(const std::string& command, const std::string& firstOperand, const std::string& secondOperand)
    : label_(""), command_(command), firstOperand_(firstOperand), secondOperand_(secondOperand)
{
}

CodeLine::CodeLine(const std::string& label, const std::string& command, const std::string& firstOperand, const std::string& secondOperand)
    : label_(label), command_(command), firstOperand_(firstOperand), secondOperand_(secondOperand)
{
}