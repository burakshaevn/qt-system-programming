#include "structures/symbolicname.h"

SymbolicName::SymbolicName()
    : name_(""), address_(0)
{
}

SymbolicName::SymbolicName(const std::string& name, int address)
    : name_(name), address_(address)
{
}