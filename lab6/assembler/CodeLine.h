#ifndef CODELINE_H
#define CODELINE_H

#include <QString>

class CodeLine
{
public:
    QString Label;  // nullable (empty string means null)
    QString Command;
    QString FirstOperand;  // nullable
    QString SecondOperand;  // nullable

    CodeLine();
    bool hasLabel() const { return !Label.isEmpty(); }
    bool hasFirstOperand() const { return !FirstOperand.isEmpty(); }
    bool hasSecondOperand() const { return !SecondOperand.isEmpty(); }
};

#endif // CODELINE_H

