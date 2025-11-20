#ifndef PARSER_H
#define PARSER_H

#include <QString>
#include <QList>
#include "assembler/CommandDto.h"
#include "assembler/AssemblerException.h"

class Parser
{
public:
    static QList<QList<QString>> ParseCode(const QString& input);
    static QList<CommandDto> TextToCommandDtos(const QString& text);
};

#endif // PARSER_H

