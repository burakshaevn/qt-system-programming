#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <QString>
#include <QList>
#include "Command.h"
#include "CommandDto.h"
#include "CodeLine.h"
#include "SymbolicName.h"
#include "AssemblerException.h"
#include "Section.h"
#include "TNLine.h"

class Assembler
{
public:
    QList<QList<QString>> SourceCode;
    QList<QString> BinaryCode;
    int lineIterator;
    QList<Command> AvailibleCommands;
    QList<SymbolicName> TSI;
    QList<TNLine> TN;  // Таблица настройки
    QString AddressingMode;  // "Straight", "Relative", "Mixed"

    Assembler();
    void SetAvailibleCommands(const QList<CommandDto>& newAvailibleCommandsDto);
    void Reset(const QList<QList<QString>>& sourceCode, const QList<CommandDto>& newCommands);
    bool ProcessStep();

private:
    static const int maxAddress = 16777215;  // 2^24 - 1
    int startAddress;
    int endAddress;
    bool startFlag;
    bool endFlag;
    int ip;
    QString previousCommand;
    QList<Section> Sections;
    Section currentSection;

    static const QStringList AvailibleDirectives;

    void ClearTSI();
    void ClearTN();
    void ClearSections();
    bool IsCommand(const QString& chunk) const;
    bool IsDirective(const QString& chunk) const;
    bool IsLabel(const QString& chunk) const;
    bool IsRelativeLabel(const QString& chunk) const;
    static bool IsXString(const QString& chunk);
    static bool IsCString(const QString& chunk);
    static bool IsRegister(const QString& chunk);
    static int GetRegisterNumber(const QString& chunk);
    SymbolicName* GetSymbolicName(const QString& chunk, const QString& section);
    static QString ConvertToASCII(const QString& chunk);
    static void OverflowCheck(int value, const QString& textLine);
    void OrderCheck(const QString& chunk, const QString& previousCommand, const QString& textLine);
    CodeLine GetCodeLineFromSource(const QList<QString>& line);
    void ProvideAddresses(SymbolicName* symbolicName);
    void DefineExtDef(SymbolicName* symbolicName);
    void PushToTSI(const QString& symbolicName, int address, const Section& section, const QString& type, const QString& textLine);
    void PushToTN(const QString& address, const QString& label, const QString& section);
    void AddSection(const Section& section);
};

#endif // ASSEMBLER_H

