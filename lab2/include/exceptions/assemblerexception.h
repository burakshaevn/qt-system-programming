#ifndef ASSEMBLEREXCEPTION_H
#define ASSEMBLEREXCEPTION_H

#include <stdexcept>
#include <string>

class AssemblerException : public std::runtime_error
{
public:
    AssemblerException();
    AssemblerException(const std::string& message);
    AssemblerException(const std::string& message, const std::exception& inner);
};

#endif // ASSEMBLEREXCEPTION_H