#include "structures/tnline.h"

TNLine::TNLine()
    : address_(""), label_(""), section_("")
{
}

TNLine::TNLine(const std::string& address, const std::string& label, const std::string& section)
    : address_(address), label_(label), section_(section)
{
}


