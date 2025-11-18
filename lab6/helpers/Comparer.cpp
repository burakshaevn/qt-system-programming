#include "Comparer.h"

bool Comparer::CompareSourceCodeVersions(const QList<QList<QString>>& list1, const QList<QList<QString>>& list2)
{
    if (list1.size() != list2.size()) {
        return false;
    }

    for (int i = 0; i < list1.size(); i++) {
        const QList<QString>& innerList1 = list1[i];
        const QList<QString>& innerList2 = list2[i];

        if (innerList1.size() != innerList2.size()) {
            return false;
        }

        for (int j = 0; j < innerList1.size(); j++) {
            if (innerList1[j] != innerList2[j]) {
                return false;
            }
        }
    }

    return true;
}

