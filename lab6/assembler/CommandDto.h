#ifndef COMMANDDTO_H
#define COMMANDDTO_H

#include <QString>

class CommandDto
{
public:
    QString Name;
    QString Code;
    QString Length;

    CommandDto();
    CommandDto(const QString& name, const QString& code, const QString& length);
};

#endif // COMMANDDTO_H

