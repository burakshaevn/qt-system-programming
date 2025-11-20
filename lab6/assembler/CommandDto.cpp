#include "CommandDto.h"

CommandDto::CommandDto()
{
}

CommandDto::CommandDto(const QString& name, const QString& code, const QString& length)
    : Name(name), Code(code), Length(length)
{
}

