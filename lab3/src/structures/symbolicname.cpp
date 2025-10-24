#include "structures/symbolicname.h"

SymbolicName::SymbolicName()
    : name_(""), address_(-1), section_(""), type_("")
{
}

SymbolicName::SymbolicName(const std::string& name, int address, const std::string& section, const std::string& type)
    : name_(name), address_(address), section_(section), type_(type)
{
}