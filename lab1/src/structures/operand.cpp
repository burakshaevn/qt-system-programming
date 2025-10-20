#include "structures/operand.h"
#include <stdexcept>

Operand::Operand()
    : value_(std::string(""))
{
}

Operand::Operand(const std::string& value)
    : value_(value)
{
}

Operand::Operand(int value)
    : value_(value)
{
}

bool Operand::isString() const
{
    return std::holds_alternative<std::string>(value_);
}

bool Operand::isInt() const
{
    return std::holds_alternative<int>(value_);
}

std::string Operand::getStringValue() const
{
    if (isString()) {
        return std::get<std::string>(value_);
    }
    throw std::runtime_error("Operand is not a string");
}

int Operand::getIntValue() const
{
    if (isInt()) {
        return std::get<int>(value_);
    }
    throw std::runtime_error("Operand is not an integer");
}