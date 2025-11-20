#ifndef COMPARER_H
#define COMPARER_H

#include <QString>
#include <QList>

class Comparer
{
public:
    static bool CompareSourceCodeVersions(const QList<QList<QString>>& list1, const QList<QList<QString>>& list2);
};

#endif // COMPARER_H

