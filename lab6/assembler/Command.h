#ifndef COMMAND_H
#define COMMAND_H

#include "CommandDto.h"
#include "AssemblerException.h"

class Command
{
public:
    QString Name;
    int Code;
    int Length;

    Command();
    Command(const CommandDto& dto);
};

#endif // COMMAND_H

