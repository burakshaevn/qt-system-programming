#ifndef SYMBOLICNAME_H
#define SYMBOLICNAME_H

#include <QString>
#include <QList>

class SymbolicName
{
public:
    QString Name;
    int Address;  // -1 means undefined
    QString Section;  // Section name
    QString Type;  // "ВИ" (EXTDEF), "ВС" (EXTREF), "AR" (Address Requirement), or "" (regular)
    QList<int> AddressRequirements;

    SymbolicName();
    bool isDefined() const { return Address != -1; }
};

#endif // SYMBOLICNAME_H

