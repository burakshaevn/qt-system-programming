#include "AssemblerException.h"

AssemblerException::AssemblerException()
    : message("Assembler error")
{
}

AssemblerException::AssemblerException(const QString& msg)
    : message(msg)
{
}

const char* AssemblerException::what() const noexcept
{
    return message.toLocal8Bit().constData();
}

