#include "Section.h"

Section::Section() : StartAddress(0), EndAddress(0), Length(0)
{
}

Section::Section(const QString& name, int startAddress)
    : Name(name), StartAddress(startAddress), EndAddress(0), Length(0)
{
}

