#include "Assembler.h"
#include <QRegularExpression>
#include <QSet>
#include <QDebug>

const QStringList Assembler::AvailibleDirectives = {"START", "END", "WORD", "BYTE", "RESB", "RESW", "EXTREF", "EXTDEF", "CSECT"};

Assembler::Assembler()
    : lineIterator(0), startAddress(0), endAddress(0), startFlag(false), endFlag(false), ip(0), AddressingMode("Straight")
{
    // Default commands
    AvailibleCommands.append(Command(CommandDto("JMP", "1", "4")));
    AvailibleCommands.append(Command(CommandDto("LOADR1", "2", "4")));
    AvailibleCommands.append(Command(CommandDto("LOADR2", "3", "4")));
    AvailibleCommands.append(Command(CommandDto("ADD", "4", "2")));
    AvailibleCommands.append(Command(CommandDto("SAVER1", "5", "4")));
    AvailibleCommands.append(Command(CommandDto("INT", "6", "2")));
    currentSection = Section();
}

void Assembler::SetAvailibleCommands(const QList<CommandDto>& newAvailibleCommandsDto)
{
    // Try to convert
    QList<Command> newAvailibleCommands;
    for (const CommandDto& dto : newAvailibleCommandsDto) {
        newAvailibleCommands.append(Command(dto));
    }

    // Check Name uniqueness
    QSet<QString> nhs;
    bool isNameUnique = true;
    for (const Command& cmd : newAvailibleCommands) {
        QString upperName = cmd.Name.toUpper();
        if (nhs.contains(upperName)) {
            isNameUnique = false;
            break;
        }
        nhs.insert(upperName);
    }

    if (!isNameUnique) {
        throw AssemblerException("Все имена команд должны быть уникальными");
    }

    bool isOverlapWithCommands = false;
    for (const Command& cmd : newAvailibleCommands) {
        if (IsDirective(cmd.Name.toUpper()) || IsRegister(cmd.Name.toUpper())) {
            isOverlapWithCommands = true;
            break;
        }
    }

    if (isOverlapWithCommands) {
        throw AssemblerException("Имена команд не должны совпадать с именами директив и регистров");
    }

    // Check Code uniqueness
    QSet<int> chs;
    bool isCodeUnique = true;
    for (const Command& cmd : newAvailibleCommands) {
        if (chs.contains(cmd.Code)) {
            isCodeUnique = false;
            break;
        }
        chs.insert(cmd.Code);
    }

    if (!isCodeUnique) {
        throw AssemblerException("Все коды команд должны быть уникальными");
    }

    this->AvailibleCommands = newAvailibleCommands;
}

void Assembler::Reset(const QList<QList<QString>>& sourceCode, const QList<CommandDto>& newCommands)
{
    SetAvailibleCommands(newCommands);
    ClearTSI();
    ClearTN();
    ClearSections();
    SourceCode = sourceCode;
    BinaryCode.clear();

    startAddress = 0;
    endAddress = 0;
    startFlag = false;
    endFlag = false;
    ip = 0;
    lineIterator = 0;
    previousCommand = "";
    currentSection = Section();
}

bool Assembler::ProcessStep()
{
    if (lineIterator == -1 || endFlag) return true;

    const QList<QString>& line = SourceCode[lineIterator];

    QString textLine = line.join(" ");
    QString binaryCodeLine;

    OverflowCheck(ip, textLine);

    CodeLine codeLine = GetCodeLineFromSource(line);

    if (SourceCode.indexOf(line) == 0) {
        if (codeLine.Command != "START") {
            throw AssemblerException("Не найдена директива START в начале программы");
        }
    } else {
        if (!startFlag) {
            throw AssemblerException("Не найдена директива START в начале программы");
        }
    }

    // Processing command part
    if (IsDirective(codeLine.Command)) {
        if (codeLine.Command == "START") {
            if (codeLine.hasSecondOperand()) {
                throw AssemblerException(QString("Ожидается один операнд, но найдено два: %1").arg(textLine));
            }

            // Start should be at the beginning and first
            if (ip != 0 || startFlag) {
                throw AssemblerException(QString("START должен быть единственным, в начале исходного кода: %1").arg(textLine));
            }

            // Check that START is the first line
            if (lineIterator != 0) {
                throw AssemblerException(QString("Программа должна начинаться с директивы START: %1").arg(textLine));
            }

            // Start was found
            startFlag = true;

            // Process first operand
            int address;

            if (codeLine.hasFirstOperand()) {
                bool ok;
                address = codeLine.FirstOperand.toInt(&ok, 10);

                if (!ok) {
                    throw AssemblerException(QString("Невозможно преобразовать первый операнд в адрес начала программы: %1").arg(textLine));
                }

                if (address != 0) {
                    throw AssemblerException(QString("Адрес загрузки должен быть равен нулю: %1").arg(textLine));
                }
            }

            address = 0;

            if (!codeLine.hasLabel()) {
                throw AssemblerException("Перед директивой START должна быть метка");
            }

            // Initialize currentSection
            currentSection = Section(codeLine.Label, address);

            ip = address;

            // Output
            binaryCodeLine = QString("H %1 %2").arg(codeLine.Label, QString::number(address, 16).toUpper().rightJustified(6, '0'));
        } else if (codeLine.Command == "CSECT") {
            if (codeLine.hasSecondOperand()) {
                throw AssemblerException(QString("Ожидается ноль или один операнд: %1").arg(textLine));
            }

            if (!codeLine.hasLabel()) {
                throw AssemblerException(QString("Перед директивой CSECT должна быть метка: %1").arg(textLine));
            }

            // Process first operand
            int endAddress;

            if (codeLine.hasFirstOperand()) {
                bool ok;
                endAddress = codeLine.FirstOperand.toInt(&ok, 10);

                if (!ok) {
                    throw AssemblerException(QString("Невозможно преобразовать первый операнд в адрес входа в секцию: %1").arg(textLine));
                }

                if (endAddress < 0 || endAddress > 16777215) {
                    throw AssemblerException(QString("Значение первого операнда выходит за границы допустимого диапазона (0-16777215): %1").arg(textLine));
                }
            } else {
                endAddress = 0;
            }

            // Update and pass the previous currentsection into Sections
            currentSection.EndAddress = endAddress;
            currentSection.Length = ip - currentSection.StartAddress;

            AddSection(currentSection);

            // Initialize new currentSection
            currentSection = Section(codeLine.Label, 0);

            BinaryCode.append(QString("E %1").arg(QString::number(endAddress, 16).toUpper().rightJustified(6, '0')));
            binaryCodeLine = QString("H %1 %2").arg(codeLine.Label, QString::number(0, 16).toUpper().rightJustified(6, '0'));

            ip = 0;
        } else if (codeLine.Command == "EXTDEF") {
            if (!codeLine.hasFirstOperand()) {
                throw AssemblerException(QString("Ожидается один операнд, но было получено ноль: %1").arg(textLine));
            }
            if (codeLine.hasSecondOperand()) {
                throw AssemblerException(QString("Ожидается один операнд, но найдено два: %1").arg(textLine));
            }

            OrderCheck(codeLine.Command, previousCommand, textLine);

            // firstOperand must be label
            if (!IsLabel(codeLine.FirstOperand)) {
                throw AssemblerException(QString("Операнд для директивы EXTDEF должен быть меткой: %1").arg(textLine));
            }

            PushToTSI(codeLine.FirstOperand, -1, currentSection, "ВИ", textLine);

            binaryCodeLine = QString("D %1 %2").arg(codeLine.FirstOperand, QString::number(0xFFFFFF, 16).toUpper().mid(2));
        } else if (codeLine.Command == "EXTREF") {
            if (!codeLine.hasFirstOperand()) {
                throw AssemblerException(QString("Ожидается один операнд, но было получено ноль: %1").arg(textLine));
            }
            if (codeLine.hasSecondOperand()) {
                throw AssemblerException(QString("Ожидается один операнд, но найдено два: %1").arg(textLine));
            }

            OrderCheck(codeLine.Command, previousCommand, textLine);

            // firstOperand must be label
            if (!IsLabel(codeLine.FirstOperand)) {
                throw AssemblerException(QString("Операнд для директивы EXTREF должен быть меткой: %1").arg(textLine));
            }

            PushToTSI(codeLine.FirstOperand, 0, currentSection, "ВС", textLine);

            binaryCodeLine = QString("R %1").arg(codeLine.FirstOperand);
        } else if (codeLine.Command == "WORD") {
            // Can only contain a 3-byte unsigned int value
            if (!codeLine.hasFirstOperand()) {
                throw AssemblerException(QString("Ожидается один операнд, но было получено ноль: %1").arg(textLine));
            }
            if (codeLine.hasSecondOperand()) {
                throw AssemblerException(QString("Ожидается один операнд, но найдено два: %1").arg(textLine));
            }

            if (codeLine.hasLabel()) {
                PushToTSI(codeLine.Label, ip, currentSection, "", textLine);
            }

            int value;
            bool ok;
            value = codeLine.FirstOperand.toInt(&ok, 10);

            if (!ok) {
                throw AssemblerException(QString("Невозможно преобразовать первый операнд в число: %1").arg(textLine));
            }

            // Check if within 0-16777215
            if (value <= 0 || value > 16777215) {
                throw AssemblerException(QString("Значение первого операнда выходит за границы допустимого диапазона (1-16777215): %1").arg(textLine));
            }

            // Check for allocated memory overflow
            OverflowCheck(ip + 3, textLine);

            binaryCodeLine = QString("T %1 %2 %3").arg(
                QString::number(ip, 16).toUpper().rightJustified(6, '0'),
                QString::number(3, 16).toUpper().rightJustified(2, '0'),
                QString::number(value, 16).toUpper().rightJustified(6, '0'));
            ip += 3;
        } else if (codeLine.Command == "BYTE") {
            if (!codeLine.hasFirstOperand()) {
                throw AssemblerException(QString("Ожидается один операнд, но было получено ноль: %1").arg(textLine));
            }
            if (codeLine.hasSecondOperand()) {
                throw AssemblerException(QString("Ожидается один операнд, но найдено два: %1").arg(textLine));
            }

            if (codeLine.hasLabel()) {
                PushToTSI(codeLine.Label, ip, currentSection, "", textLine);
            }

            int value;
            bool ok;

            // Try to parse as a 1 byte value
            value = codeLine.FirstOperand.toInt(&ok, 10);
            if (ok) {
                // Check if within 0-255
                if (value < 0 || value > 255) {
                    throw AssemblerException(QString("Значение первого операнда выходит за границы допустимого диапазона (0-255): %1").arg(textLine));
                }

                // Check for allocated memory overflow
                OverflowCheck(ip + 1, textLine);

                binaryCodeLine = QString("T %1 %2 %3").arg(
                    QString::number(ip, 16).toUpper().rightJustified(6, '0'),
                    QString::number(1, 16).toUpper().rightJustified(2, '0'),
                    QString::number(value, 16).toUpper().rightJustified(2, '0'));
                ip += 1;
            } else if (IsCString(codeLine.FirstOperand)) {
                // Couldn't parse as a numeric value => parse as a character string
                QString symbols = codeLine.FirstOperand.mid(2, codeLine.FirstOperand.length() - 3);

                if (symbols.length() > 255) {
                    throw AssemblerException(QString("Длина строки не может превышать 255 байт: %1").arg(textLine));
                }

                // Check for allocated memory overflow
                OverflowCheck(ip + symbols.length(), textLine);

                binaryCodeLine = QString("T %1 %2 %3").arg(
                    QString::number(ip, 16).toUpper().rightJustified(6, '0'),
                    QString::number(symbols.length(), 16).toUpper().rightJustified(2, '0'),
                    ConvertToASCII(symbols));
                ip += symbols.length();
            } else if (IsXString(codeLine.FirstOperand)) {
                QString symbols = codeLine.FirstOperand.mid(2, codeLine.FirstOperand.length() - 3);

                if (symbols.length() / 2 > 255) {
                    throw AssemblerException(QString("Длина строки не может превышать 255 байт: %1").arg(textLine));
                }

                // Check for allocated memory overflow
                OverflowCheck(ip + symbols.length() / 2, textLine);

                binaryCodeLine = QString("T %1 %2 %3").arg(
                    QString::number(ip, 16).toUpper().rightJustified(6, '0'),
                    QString::number(symbols.length() / 2, 16).toUpper().rightJustified(2, '0'),
                    symbols);
                ip += symbols.length() / 2;
            } else {
                throw AssemblerException(QString("Невозможно преобразовать первый операнд в символьную или шестнадцатеричную строку: %1").arg(textLine));
            }
        } else if (codeLine.Command == "RESW") {
            if (!codeLine.hasFirstOperand()) {
                throw AssemblerException(QString("Ожидается один операнд, но было получено ноль: %1").arg(textLine));
            }
            if (codeLine.hasSecondOperand()) {
                throw AssemblerException(QString("Ожидается один операнд, но найдено два: %1").arg(textLine));
            }

            if (codeLine.hasLabel()) {
                PushToTSI(codeLine.Label, ip, currentSection, "", textLine);
            }

            int value;
            bool ok;
            value = codeLine.FirstOperand.toInt(&ok, 10);

            if (!ok) {
                throw AssemblerException(QString("Невозможно преобразовать первый операнд в число: %1").arg(textLine));
            }

            // Check if within 0-255
            if (value <= 0 || value > 255) {
                throw AssemblerException(QString("Значение первого операнда выходит за границы допустимого диапазона (1-255): %1").arg(textLine));
            }

            // Check for allocated memory overflow
            OverflowCheck(ip + value * 3, textLine);

            binaryCodeLine = QString("T %1 %2").arg(
                QString::number(ip, 16).toUpper().rightJustified(6, '0'),
                QString::number(value * 3, 16).toUpper().rightJustified(2, '0'));
            ip += value * 3;
        } else if (codeLine.Command == "RESB") {
            if (!codeLine.hasFirstOperand()) {
                throw AssemblerException(QString("Ожидается один операнд, но было получено ноль: %1").arg(textLine));
            }
            if (codeLine.hasSecondOperand()) {
                throw AssemblerException(QString("Ожидается один операнд, но найдено два: %1").arg(textLine));
            }

            if (codeLine.hasLabel()) {
                PushToTSI(codeLine.Label, ip, currentSection, "", textLine);
            }

            int value;
            bool ok;
            value = codeLine.FirstOperand.toInt(&ok, 10);

            if (!ok) {
                throw AssemblerException(QString("Невозможно преобразовать первый операнд в число: %1").arg(textLine));
            }

            // Check if within 0-16777215
            if (value <= 0 || value > 255) {
                throw AssemblerException(QString("Значение первого операнда выходит за границы допустимого диапазона (1-255): %1").arg(textLine));
            }

            // Check for allocated memory overflow
            OverflowCheck(ip + value, textLine);

            binaryCodeLine = QString("T %1 %2").arg(
                QString::number(ip, 16).toUpper().rightJustified(6, '0'),
                QString::number(value, 16).toUpper().rightJustified(2, '0'));
            ip += value;
        } else if (codeLine.Command == "END") {
            if (codeLine.hasSecondOperand()) {
                throw AssemblerException(QString("Ожидается максимум один операнд, но найдено два: %1").arg(textLine));
            }

            if (!startFlag || endFlag) {
                throw AssemblerException(QString("Не найдена метка START либо ошибка в директивах START/END: %1").arg(textLine));
            }

            if (codeLine.hasLabel()) {
                PushToTSI(codeLine.Label, ip, currentSection, "", textLine);
            }

            // Process first operand
            int endAddress;

            if (!codeLine.hasFirstOperand()) {
                endAddress = 0;
            } else {
                bool ok;
                endAddress = codeLine.FirstOperand.toInt(&ok, 10);

                if (!ok) {
                    throw AssemblerException(QString("Невозможно преобразовать первый операнд в адрес входа в секцию: %1").arg(textLine));
                }

                if (endAddress < 0 || endAddress > 16777215) {
                    throw AssemblerException(QString("Значение первого операнда выходит за границы допустимого диапазона (0-16777215): %1").arg(textLine));
                }
            }

            // Update and pass the previous currentsection into Sections
            currentSection.EndAddress = endAddress;
            currentSection.Length = ip - currentSection.StartAddress;

            AddSection(currentSection);

            // Output
            binaryCodeLine = QString("E %1").arg(QString::number(endAddress, 16).toUpper().rightJustified(6, '0'));

            endFlag = true;
        }
    } else if (IsCommand(codeLine.Command)) {
        if (codeLine.hasLabel()) {
            PushToTSI(codeLine.Label, ip, currentSection, "", textLine);
        }

        Command* command = nullptr;
        for (int i = 0; i < AvailibleCommands.size(); i++) {
            if (AvailibleCommands[i].Name.toUpper() == codeLine.Command.toUpper()) {
                command = &AvailibleCommands[i];
                break;
            }
        }

        if (command == nullptr) {
            throw AssemblerException(QString("Команда не найдена: %1").arg(textLine));
        }

        OverflowCheck(ip + command->Length, textLine);

        if (command->Length == 1) {
            // Length is 1 (operandless)
            if (codeLine.hasFirstOperand()) {
                throw AssemblerException(QString("Ожидается ноль операндов: %1").arg(textLine));
            }

            // Addressing type 00
            binaryCodeLine = QString("T %1 %2 %3").arg(
                QString::number(ip, 16).toUpper().rightJustified(6, '0'),
                QString::number(command->Length, 16).toUpper().rightJustified(2, '0'),
                QString::number(command->Code * 4 + 0, 16).toUpper().rightJustified(2, '0'));
        } else if (command->Length == 2) {
            // Length is 2
            // Either two registers as two operands
            // or one 1-byte value
            if (!codeLine.hasFirstOperand()) {
                throw AssemblerException(QString("Ожидается минимум один операнд, но было получено ноль: %1").arg(textLine));
            }

            // Two registers
            if (codeLine.hasSecondOperand()) {
                if (IsRegister(codeLine.FirstOperand) && IsRegister(codeLine.SecondOperand)) {
                    // Addressing type 00
                    binaryCodeLine = QString("T %1 %2 %3%4%5").arg(
                        QString::number(ip, 16).toUpper().rightJustified(6, '0'),
                        QString::number(command->Length, 16).toUpper().rightJustified(2, '0'),
                        QString::number(command->Code * 4 + 0, 16).toUpper().rightJustified(2, '0'),
                        QString::number(GetRegisterNumber(codeLine.FirstOperand), 16).toUpper().rightJustified(1, '0'),
                        QString::number(GetRegisterNumber(codeLine.SecondOperand), 16).toUpper().rightJustified(1, '0'));
                } else {
                    throw AssemblerException(QString("Неверный формат команды. Ожидалось два регистра: %1").arg(textLine));
                }
            } else {
                // 1-byte value
                int value;
                bool ok;
                value = codeLine.FirstOperand.toInt(&ok, 10);

                if (!ok) {
                    throw AssemblerException(QString("Невозможно преобразовать первый операнд в число: %1").arg(textLine));
                }

                // Check if within 0-255
                if (value < 0 || value > 255) {
                    throw AssemblerException(QString("Значение первого операнда выходит за границы допустимого диапазона (0-255): %1").arg(textLine));
                }

                // Addressing type 00
                binaryCodeLine = QString("T %1 %2 %3%4").arg(
                    QString::number(ip, 16).toUpper().rightJustified(6, '0'),
                    QString::number(command->Length, 16).toUpper().rightJustified(2, '0'),
                    QString::number(command->Code * 4 + 0, 16).toUpper().rightJustified(2, '0'),
                    QString::number(value, 16).toUpper().rightJustified(2, '0'));
            }
        } else if (command->Length == 4) {
            // Length 4
            if (!codeLine.hasFirstOperand()) {
                throw AssemblerException(QString("Ожидается один операнд, но было получено ноль: %1").arg(textLine));
            }
            if (codeLine.hasSecondOperand()) {
                throw AssemblerException(QString("Ожидается один операнд, но найдено два: %1").arg(textLine));
            }

            // Is it a relative label?
            if (IsRelativeLabel(codeLine.FirstOperand)) {
                if (AddressingMode == "Straight") {
                    throw AssemblerException(QString("Данный тип адресации недоступен в этом режиме адресации: %1").arg(textLine));
                }

                QString label = codeLine.FirstOperand.mid(1, codeLine.FirstOperand.length() - 2);

                SymbolicName* sn = GetSymbolicName(label, currentSection.Name);

                if (sn == nullptr) {
                    PushToTSI(label, ip, currentSection, "AR", textLine);
                    QString negativeOne = QString::number(0xFFFFFF, 16).toUpper().rightJustified(6, '0');
                    binaryCodeLine = QString("T %1 %2 %3%4").arg(
                        QString::number(ip, 16).toUpper().rightJustified(6, '0'),
                        QString::number(command->Length, 16).toUpper().rightJustified(2, '0'),
                        QString::number(command->Code * 4 + 2, 16).toUpper().rightJustified(2, '0'),
                        negativeOne);
                } else if (sn->Address == -1) {
                    PushToTSI(label, ip, currentSection, "AR", textLine);
                    QString negativeOne = QString::number(0xFFFFFF, 16).toUpper().rightJustified(6, '0');
                    binaryCodeLine = QString("T %1 %2 %3%4").arg(
                        QString::number(ip, 16).toUpper().rightJustified(6, '0'),
                        QString::number(command->Length, 16).toUpper().rightJustified(2, '0'),
                        QString::number(command->Code * 4 + 2, 16).toUpper().rightJustified(2, '0'),
                        negativeOne);
                } else {
                    if (sn->Type == "ВС") {
                        throw AssemblerException(QString("Относительный тип адресации недоступен для внешних ссылок: %1").arg(textLine));
                    } else {
                        int relativeOffset = sn->Address - (ip + command->Length);
                        QString offsetStr;
                        if (relativeOffset < 0) {
                            // Используем полный 6-символьный формат для отрицательных offset
                            offsetStr = QString::number(relativeOffset & 0xFFFFFF, 16).toUpper().rightJustified(6, '0');
                        } else {
                            offsetStr = QString::number(relativeOffset, 16).toUpper().rightJustified(6, '0');
                        }
                        binaryCodeLine = QString("T %1 %2 %3%4").arg(
                            QString::number(ip, 16).toUpper().rightJustified(6, '0'),
                            QString::number(command->Length, 16).toUpper().rightJustified(2, '0'),
                            QString::number(command->Code * 4 + 2, 16).toUpper().rightJustified(2, '0'),
                            offsetStr);
                    }
                }
            } else if (IsLabel(codeLine.FirstOperand)) {
                if (AddressingMode == "Relative") {
                    throw AssemblerException(QString("Данный тип адресации недоступен в этом режиме адресации: %1").arg(textLine));
                }

                SymbolicName* sn = GetSymbolicName(codeLine.FirstOperand, currentSection.Name);

                if (sn == nullptr) {
                    PushToTSI(codeLine.FirstOperand, ip, currentSection, "AR", textLine);
                    QString negativeOne = QString::number(0xFFFFFF, 16).toUpper().rightJustified(6, '0');
                    binaryCodeLine = QString("T %1 %2 %3%4").arg(
                        QString::number(ip, 16).toUpper().rightJustified(6, '0'),
                        QString::number(command->Length, 16).toUpper().rightJustified(2, '0'),
                        QString::number(command->Code * 4 + 1, 16).toUpper().rightJustified(2, '0'),
                        negativeOne);
                    PushToTN(QString::number(ip, 16).toUpper().rightJustified(6, '0'), "", currentSection.Name);
                } else if (sn->Address == -1) {
                    PushToTSI(codeLine.FirstOperand, ip, currentSection, "AR", textLine);
                    QString negativeOne = QString::number(0xFFFFFF, 16).toUpper().rightJustified(6, '0');
                    binaryCodeLine = QString("T %1 %2 %3%4").arg(
                        QString::number(ip, 16).toUpper().rightJustified(6, '0'),
                        QString::number(command->Length, 16).toUpper().rightJustified(2, '0'),
                        QString::number(command->Code * 4 + 1, 16).toUpper().rightJustified(2, '0'),
                        negativeOne);
                    PushToTN(QString::number(ip, 16).toUpper().rightJustified(6, '0'), "", currentSection.Name);
                } else {
                    if (sn->Type == "ВС") {
                        binaryCodeLine = QString("T %1 %2 %3%4").arg(
                            QString::number(ip, 16).toUpper().rightJustified(6, '0'),
                            QString::number(command->Length, 16).toUpper().rightJustified(2, '0'),
                            QString::number(command->Code * 4 + 1, 16).toUpper().rightJustified(2, '0'),
                            QString::number(sn->Address, 16).toUpper().rightJustified(6, '0'));
                        PushToTN(QString::number(ip, 16).toUpper().rightJustified(6, '0'), sn->Name, currentSection.Name);
                    } else {
                        binaryCodeLine = QString("T %1 %2 %3%4").arg(
                            QString::number(ip, 16).toUpper().rightJustified(6, '0'),
                            QString::number(command->Length, 16).toUpper().rightJustified(2, '0'),
                            QString::number(command->Code * 4 + 1, 16).toUpper().rightJustified(2, '0'),
                            QString::number(sn->Address, 16).toUpper().rightJustified(6, '0'));
                        PushToTN(QString::number(ip, 16).toUpper().rightJustified(6, '0'), "", currentSection.Name);
                    }
                }
            } else {
                // Is it a parsable 3-byte value?
                bool ok;
                int value = codeLine.FirstOperand.toInt(&ok, 10);
                if (ok) {
                    if (value < 0 || value > 16777215) {
                        throw AssemblerException(QString("Недопустимое значение операнда: %1").arg(textLine));
                    }

                    // Addressing type 00
                    binaryCodeLine = QString("T %1 %2 %3%4").arg(
                        QString::number(ip, 16).toUpper().rightJustified(6, '0'),
                        QString::number(command->Length, 16).toUpper().rightJustified(2, '0'),
                        QString::number(command->Code * 4, 16).toUpper().rightJustified(2, '0'),
                        QString::number(value, 16).toUpper().rightJustified(6, '0'));
                } else {
                    throw AssemblerException(QString("Недопустимое значение операнда: %1").arg(textLine));
                }
            }
        }

        ip += command->Length;
    } else {
        throw AssemblerException(QString("Неизвестная команда: %1").arg(textLine));
    }

    BinaryCode.append(binaryCodeLine);

    previousCommand = codeLine.Command;

    lineIterator++;

    if (lineIterator >= SourceCode.size()) {
        lineIterator = -1;
    }

    if (lineIterator == -1 && !endFlag) {
        throw AssemblerException("Не найдена точка входа в программу.");
    }

    return false;
}

void Assembler::OrderCheck(const QString& chunk, const QString& previousCommand, const QString& textLine)
{
    if (chunk == "EXTDEF") {
        if (previousCommand != "START" && previousCommand != "CSECT" && previousCommand != "EXTDEF") {
            throw AssemblerException(QString("Директива EXTDEF может стоять только после директив START, CSECT и EXTDEF: %1").arg(textLine));
        }
    } else if (chunk == "EXTREF") {
        if (previousCommand != "START" && previousCommand != "CSECT" && previousCommand != "EXTDEF" && previousCommand != "EXTREF") {
            throw AssemblerException(QString("Директива EXTREF может стоять только после директив START, CSECT, EXTDEF и EXTREF: %1").arg(textLine));
        }
    }
}

void Assembler::ProvideAddresses(SymbolicName* symbolicName)
{
    // Find start line index for the section
    int startLineIndex = -1;
    for (int i = 0; i < BinaryCode.size(); i++) {
        QStringList parts = BinaryCode[i].split(' ');
        if (parts.size() >= 2 && parts[0] == "H" && parts[1] == symbolicName->Section) {
            startLineIndex = i;
            break;
        }
    }

    if (startLineIndex == -1) {
        return;
    }

    // Find end line index for this section (next H line or end of BinaryCode)
    int endLineIndex = BinaryCode.size();
    for (int i = startLineIndex + 1; i < BinaryCode.size(); i++) {
        QStringList parts = BinaryCode[i].split(' ');
        if (parts.size() >= 2 && parts[0] == "H") {
            endLineIndex = i;
            break;
        }
    }

    for (int requirement : symbolicName->AddressRequirements) {
        // Find the T line with matching address within this section
        QString matchingLine;
        int index = -1;
        for (int i = startLineIndex + 1; i < endLineIndex; i++) {
            QString line = BinaryCode[i];
            QStringList parts = line.split(' ');
            if (parts.size() >= 2 && parts[0] == "T") {
                bool ok;
                int address = parts[1].toInt(&ok, 16);
                if (ok && address == requirement) {
                    matchingLine = line;
                    index = i;
                    break;
                }
            }
        }

        if (index == -1 || matchingLine.isEmpty()) {
            continue;
        }

        QStringList parts = matchingLine.split(' ');
        if (parts.size() >= 4) {
            QString lengthStr = parts[2];
            QString commandData = parts[3];
            if (commandData.length() >= 2) {
                QString commandByte = commandData.left(2);
                bool addrOk;
                int addrType = commandByte.toInt(&addrOk, 16);
                if (addrOk) {
                    int addressingType = addrType & 0x03;

                    switch (addressingType) {
                        case 1:  // Straight addressing
                        {
                            QString newAddress = QString::number(symbolicName->Address, 16).toUpper().rightJustified(6, '0');
                            QString newLine = matchingLine;
                            // Replace last 6 hex digits
                            newLine = newLine.left(newLine.length() - 6) + newAddress;
                            BinaryCode[index] = newLine;
                            break;
                        }
                        case 2:  // Relative addressing
                        {
                            bool ok;
                            int ip = parts[1].toInt(&ok, 16);
                            int length = lengthStr.toInt(&ok, 16);

                            int relativeOffset = symbolicName->Address - (ip + length);
                            QString offsetStr;
                            if (relativeOffset < 0) {
                                // Используем полный 6-символьный формат для отрицательных offset
                                offsetStr = QString::number(relativeOffset & 0xFFFFFF, 16).toUpper().rightJustified(6, '0');
                            } else {
                                offsetStr = QString::number(relativeOffset, 16).toUpper().rightJustified(6, '0');
                            }

                            // Remove last 6 characters and replace with new offset
                            // Format: "T <address> <length> <command(2)><offset(6)>"
                            // We remove last 6 chars (old offset) and add new offset
                            QString newLine = matchingLine;
                            newLine = matchingLine.left(matchingLine.length() - 6) + offsetStr;
                            BinaryCode[index] = newLine;
                            break;
                        }
                    }
                }
            }
        }
    }

    symbolicName->AddressRequirements.clear();
}

void Assembler::PushToTSI(const QString& symbolicName, int address, const Section& section, const QString& type, const QString& textLine)
{
    QString upperName = symbolicName.toUpper();
    SymbolicName* sn = nullptr;

    for (int i = 0; i < TSI.size(); i++) {
        if (TSI[i].Section == section.Name && TSI[i].Name.toUpper() == upperName) {
            sn = &TSI[i];
            break;
        }
    }

    if (type == "ВИ" || type == "ВС") {
        // Called from EXTDEF/EXTREF
        if (sn == nullptr) {
            SymbolicName newSymbolicName;
            newSymbolicName.Name = upperName;
            newSymbolicName.Address = address;
            newSymbolicName.Section = section.Name;
            newSymbolicName.Type = type;
            TSI.append(newSymbolicName);
        } else {
            throw AssemblerException(QString("Такая метка уже есть в ТСИ: %1").arg(textLine));
        }
    } else if (type == "AR") {
        // Called from operand part
        if (sn == nullptr) {
            SymbolicName newSymbolicName;
            newSymbolicName.Name = upperName;
            newSymbolicName.Address = -1;
            newSymbolicName.Section = section.Name;
            newSymbolicName.Type = "";
            newSymbolicName.AddressRequirements.append(address);
            TSI.append(newSymbolicName);
        } else {
            if (sn->Address == -1) {
                sn->AddressRequirements.append(address);
            }
        }
    } else {
        // type == "", called from label part
        if (sn == nullptr) {
            SymbolicName newSymbolicName;
            newSymbolicName.Name = upperName;
            newSymbolicName.Address = address;
            newSymbolicName.Section = section.Name;
            newSymbolicName.Type = "";
            TSI.append(newSymbolicName);
        } else {
            if (sn->Type == "ВИ") {
                if (sn->Address == -1) {
                    sn->Address = address;
                    ProvideAddresses(sn);
                    DefineExtDef(sn);
                } else {
                    throw AssemblerException(QString("Такая метка уже есть в ТСИ: %1").arg(textLine));
                }
            } else if (sn->Type == "ВС") {
                throw AssemblerException(QString("Такая метка уже есть в ТСИ: %1").arg(textLine));
            } else {
                if (sn->Address == -1) {
                    sn->Address = address;
                    ProvideAddresses(sn);
                } else {
                    throw AssemblerException(QString("Такая метка уже есть в ТСИ: %1").arg(textLine));
                }
            }
        }
    }
}

void Assembler::PushToTN(const QString& address, const QString& label, const QString& section)
{
    TNLine tnLine;
    tnLine.Address = address;
    tnLine.Label = label;
    tnLine.Section = section;
    TN.append(tnLine);
}

void Assembler::DefineExtDef(SymbolicName* symbolicName)
{
    // Find D line and update it
    for (int i = 0; i < BinaryCode.size(); i++) {
        QStringList parts = BinaryCode[i].split(' ');
        if (parts.size() >= 2 && parts[0] == "D" && parts[1] == symbolicName->Name) {
            BinaryCode[i] = QString("D %1 %2").arg(symbolicName->Name, QString::number(symbolicName->Address, 16).toUpper().rightJustified(6, '0'));
            break;
        }
    }
}

void Assembler::AddSection(const Section& section)
{
    // Check if all external definitions in this section have addresses
    for (const SymbolicName& sn : TSI) {
        if (sn.Section == section.Name && sn.Type == "ВИ" && sn.Address == -1) {
            throw AssemblerException("Не всем внешним именам было присвоено значение");
        }
    }

    // Check if all symbolic names in this section have addresses
    for (const SymbolicName& sn : TSI) {
        if (sn.Section == section.Name && sn.Type != "ВС" && !sn.AddressRequirements.isEmpty()) {
            throw AssemblerException("Не всем символическим именам было присвоено значение");
        }
    }

    // Check if section name is unique
    for (const Section& s : Sections) {
        if (s.Name == section.Name) {
            throw AssemblerException(QString("Все имена секций должны быть уникальными: %1").arg(section.Name));
        }
    }

    // Check if end address is valid
    if (section.EndAddress < section.StartAddress || section.EndAddress > section.StartAddress + section.Length) {
        throw AssemblerException(QString("Точка входа в секцию выходит за границы: %1").arg(section.Name));
    }

    // Check total length
    int totalLength = 0;
    for (const Section& s : Sections) {
        totalLength += s.Length;
    }
    OverflowCheck(totalLength + section.Length, section.Name);

    // Add M records from TN for this section
    for (const TNLine& m : TN) {
        if (m.Section == section.Name) {
            QString labelPart = m.Label.isEmpty() ? " " : QString(" %1").arg(m.Label);
            BinaryCode.append(QString("M %1%2").arg(m.Address, labelPart));
        }
    }

    // Update H line with length
    for (int i = 0; i < BinaryCode.size(); i++) {
        QStringList parts = BinaryCode[i].split(' ');
        if (parts.size() >= 2 && parts[0] == "H" && parts[1] == section.Name) {
            BinaryCode[i] = QString("%1 %2").arg(BinaryCode[i], QString::number(section.Length, 16).toUpper().rightJustified(6, '0'));
            break;
        }
    }

    Sections.append(section);
}

void Assembler::ClearTSI()
{
    TSI.clear();
}

void Assembler::ClearTN()
{
    TN.clear();
}

void Assembler::ClearSections()
{
    Sections.clear();
    currentSection = Section();
}

bool Assembler::IsCommand(const QString& chunk) const
{
    if (chunk.isEmpty()) return false;

    QString upperChunk = chunk.toUpper();
    for (const Command& cmd : AvailibleCommands) {
        if (cmd.Name.toUpper() == upperChunk) {
            return true;
        }
    }
    return false;
}

bool Assembler::IsDirective(const QString& chunk) const
{
    if (chunk.isEmpty()) return false;

    return AvailibleDirectives.contains(chunk.toUpper());
}

bool Assembler::IsLabel(const QString& chunk) const
{
    if (chunk.isEmpty()) return false;

    if (chunk.length() > 10) return false;

    QString firstChar = chunk.left(1);
    if (!firstChar.contains(QRegularExpression("[a-zA-Z]"))) return false;

    QRegularExpression labelRegex("^[a-zA-Z0-9_]+$");
    if (!labelRegex.match(chunk).hasMatch()) return false;

    if (IsRegister(chunk.toUpper())) return false;

    if (IsCommand(chunk) || IsDirective(chunk)) return false;

    return true;
}

bool Assembler::IsRelativeLabel(const QString& chunk) const
{
    if (chunk.isEmpty()) return false;

    if (chunk.length() < 3) return false;

    if (!chunk.startsWith('[') || !chunk.endsWith(']')) return false;

    QString symbols = chunk.mid(1, chunk.length() - 2);

    if (IsLabel(symbols)) return true;

    return false;
}

bool Assembler::IsXString(const QString& chunk)
{
    if (chunk.isEmpty()) return false;

    if (!chunk.startsWith("X\"", Qt::CaseInsensitive) || !chunk.endsWith("\"")) {
        return false;
    }

    QString symbols = chunk.mid(2, chunk.length() - 3).toUpper();

    if (symbols.length() < 1 || symbols.contains("\"") || symbols.length() % 2 != 0) {
        return false;
    }

    QRegularExpression hexRegex("^[0-9A-F]+$");
    if (!hexRegex.match(symbols).hasMatch()) {
        return false;
    }

    return true;
}

bool Assembler::IsCString(const QString& chunk)
{
    if (chunk.isEmpty()) return false;

    if (!chunk.startsWith("C\"", Qt::CaseInsensitive) || !chunk.endsWith("\"") || chunk.length() < 4) {
        return false;
    }

    QString symbols = chunk.mid(1, chunk.length() - 1);

    if (symbols.length() < 1) {
        return false;
    }

    // Check if all characters are ASCII (0-127)
    for (QChar c : symbols) {
        if (c.unicode() > 127) {
            return false;
        }
    }

    return true;
}

bool Assembler::IsRegister(const QString& chunk)
{
    if (chunk.isEmpty()) return false;

    QRegularExpression regex("^R([1-9]|1[0-6])$");
    return regex.match(chunk).hasMatch();
}

int Assembler::GetRegisterNumber(const QString& chunk)
{
    QString numberStr = chunk.mid(1);
    return numberStr.toInt() - 1;
}

SymbolicName* Assembler::GetSymbolicName(const QString& chunk, const QString& section)
{
    QString upperChunk = chunk.toUpper();
    for (int i = 0; i < TSI.size(); i++) {
        if (TSI[i].Name.toUpper() == upperChunk && TSI[i].Section == section) {
            return &TSI[i];
        }
    }
    return nullptr;
}

QString Assembler::ConvertToASCII(const QString& chunk)
{
    QString result;
    QByteArray textBytes = chunk.toLatin1();
    for (int i = 0; i < textBytes.length(); i++) {
        result += QString::number(static_cast<unsigned char>(textBytes[i]), 16).toUpper().rightJustified(2, '0');
    }
    return result;
}

void Assembler::OverflowCheck(int value, const QString& textLine)
{
    if (value < 0 || value > maxAddress) {
        throw AssemblerException(QString("Выход за границы выделенной памяти: %1").arg(textLine));
    }
}

CodeLine Assembler::GetCodeLineFromSource(const QList<QString>& line)
{
    QString textLine = line.join(" ");

    if (line.size() < 1 || line.size() > 4) {
        throw AssemblerException(QString("Неверный формат команды: %1").arg(textLine));
    }

    CodeLine codeLine;

    switch (line.size()) {
        case 1:
            // Can only be an operand-less command or END
            if (IsCommand(line[0]) || line[0].toUpper() == "END") {
                codeLine.Label = "";
                codeLine.Command = line[0].toUpper();
                codeLine.FirstOperand = "";
                codeLine.SecondOperand = "";
            } else {
                throw AssemblerException(QString("Неверный формат команды: %1").arg(textLine));
            }
            break;

        case 2:
            // Can be a label and an operand-less command or start/end/csect
            if (IsRegister(line[0].toUpper())) {
                throw AssemblerException(QString("Регистр не может использоваться в качестве метки: %1").arg(textLine));
            } else if (IsLabel(line[0]) && (IsCommand(line[1]) || line[1].toUpper() == "START" || line[1].toUpper() == "END" || line[1].toUpper() == "CSECT")) {
                codeLine.Label = line[0].toUpper();
                codeLine.Command = line[1].toUpper();
                codeLine.FirstOperand = "";
                codeLine.SecondOperand = "";
            } else if (IsCommand(line[0]) || (IsDirective(line[0]) && line[0].toUpper() != "CSECT")) {
                // Can be a command with one operand
                // or a keyword with one operand
                codeLine.Label = "";
                codeLine.Command = line[0].toUpper();
                codeLine.FirstOperand = line[1];
                codeLine.SecondOperand = "";
            } else {
                throw AssemblerException(QString("Неверный формат команды: %1").arg(textLine));
            }
            break;

        case 3:
            // Can be a label and a keyword with one operand
            // can be a command with two operands
            if (IsRegister(line[0].toUpper())) {
                throw AssemblerException(QString("Регистр не может использоваться в качестве метки: %1").arg(textLine));
            } else if (IsLabel(line[0]) && (IsCommand(line[1]) || (IsDirective(line[1]) && line[1].toUpper() != "EXTDEF" && line[1].toUpper() != "EXTREF"))) {
                codeLine.Label = line[0].toUpper();
                codeLine.Command = line[1].toUpper();
                codeLine.FirstOperand = line[2];
                codeLine.SecondOperand = "";
            } else if (IsCommand(line[0])) {
                codeLine.Label = "";
                codeLine.Command = line[0].toUpper();
                codeLine.FirstOperand = line[1];
                codeLine.SecondOperand = line[2];
            } else {
                throw AssemblerException(QString("Неверный формат команды: %1").arg(textLine));
            }
            break;

        case 4:
            // Can only be a label and a command and two operands
            if (IsRegister(line[0].toUpper())) {
                throw AssemblerException(QString("Регистр не может использоваться в качестве метки: %1").arg(textLine));
            } else if (IsLabel(line[0]) && IsCommand(line[1])) {
                codeLine.Label = line[0].toUpper();
                codeLine.Command = line[1].toUpper();
                codeLine.FirstOperand = line[2];
                codeLine.SecondOperand = line[3];
            } else {
                throw AssemblerException(QString("Неверный формат команды: %1").arg(textLine));
            }
            break;

        default:
            throw AssemblerException(QString("Неверный формат команды. Ни один из известных форматов не применим: %1").arg(textLine));
    }

    return codeLine;
}

