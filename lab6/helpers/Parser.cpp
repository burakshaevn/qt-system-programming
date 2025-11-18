#include "Parser.h"
#include <QRegularExpression>
#include <QStringList>

QList<QList<QString>> Parser::ParseCode(const QString& input)
{
    QStringList lines = input.split(QRegularExpression("\\r?\\n"), Qt::SkipEmptyParts);
    QList<QList<QString>> result;

    for (const QString& line : lines) {
        QString lineWithoutTabs = line;
        lineWithoutTabs.replace('\t', ' ');

        // Pattern to match words and C/X strings
        QRegularExpression pattern("((?:[CX])\"[^\"]*(?:\"[^\"]*)*\"|\\S+)");
        QRegularExpressionMatchIterator matches = pattern.globalMatch(lineWithoutTabs);

        QList<QString> words;
        while (matches.hasNext()) {
            QRegularExpressionMatch match = matches.next();
            QString word = match.captured(0).trimmed();
            if (!word.isEmpty()) {
                words.append(word);
            }
        }

        if (!words.isEmpty()) {
            result.append(words);
        }
    }

    return result;
}

QList<CommandDto> Parser::TextToCommandDtos(const QString& text)
{
    QList<QList<QString>> lines = ParseCode(text);

    // Check if there is the right amount of chunks in a line
    for (const QList<QString>& line : lines) {
        if (line.size() != 3) {
            throw AssemblerException(QString("Неправильный формат строки: %1").arg(line.join(" ")));
        }
    }

    QList<CommandDto> commandDtos;
    for (const QList<QString>& line : lines) {
        commandDtos.append(CommandDto(line[0], line[1], line[2]));
    }

    return commandDtos;
}

