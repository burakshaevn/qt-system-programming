#include "structures/section.h"

Section::Section()
    : name_(""), startAddress_(0), endAddress_(0), length_(0)
{
}

Section::Section(const std::string& name, int startAddress, int endAddress, int length)
    : name_(name), startAddress_(startAddress), endAddress_(endAddress), length_(length)
{
}


