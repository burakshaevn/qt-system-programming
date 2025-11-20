#ifndef SECTION_H
#define SECTION_H

#include <QString>

class Section
{
public:
    QString Name;
    int StartAddress;
    int EndAddress;
    int Length;

    Section();
    Section(const QString& name, int startAddress);
};

#endif // SECTION_H

