#ifndef TNLINE_H
#define TNLINE_H

#include <QString>

class TNLine
{
public:
    QString Address;
    QString Label;  // empty means no label
    QString Section;

    TNLine();
    TNLine(const QString& address, const QString& label, const QString& section);
};

#endif // TNLINE_H

