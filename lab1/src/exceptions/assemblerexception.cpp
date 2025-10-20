#include "exceptions/assemblerexception.h"

AssemblerException::AssemblerException()
    : std::runtime_error("Assembler error")
{
}

AssemblerException::AssemblerException(const std::string& message)
    : std::runtime_error(message)
{
}

AssemblerException::AssemblerException(const std::string& message, const std::exception& inner)
    : std::runtime_error(message + ": " + inner.what())
{
}