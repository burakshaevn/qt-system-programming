#include "Command.h"
#include <QString>
#include <QRegularExpression>

Command::Command()
{
}

Command::Command(const CommandDto& dto)
{
    QString command = QString("%1 %2 %3").arg(dto.Name, dto.Code, dto.Length);

    // Name validation
    if (dto.Name.isEmpty()) {
        throw AssemblerException(QString("Название команды должно содержать как минимум один символ: %1").arg(command));
    }

    QString firstChar = dto.Name.left(1);
    if (!firstChar.contains(QRegularExpression("[a-zA-Z]"))) {
        throw AssemblerException(QString("Название команды должно начинаться с латинской буквы: %1").arg(command));
    }

    QRegularExpression nameRegex("^[a-zA-Z0-9]+$");
    if (!nameRegex.match(dto.Name).hasMatch()) {
        throw AssemblerException(QString("Название команды должно состоять из латинских букв и цифр: %1").arg(command));
    }

    Name = dto.Name;

    // Code validation
    bool ok;
    int code = dto.Code.toInt(&ok, 16);
    if (!ok) {
        throw AssemblerException(QString("Код команды должен быть целым числом в 16-ричном формате: %1").arg(command));
    }

    if (code < 0 || code >= 64) {
        throw AssemblerException(QString("Код команды должен быть значением от 0 до 63: %1").arg(command));
    }

    Code = code;

    // Length validation
    int length = dto.Length.toInt(&ok, 16);
    if (!ok) {
        throw AssemblerException(QString("Длина команды должна быть целым числом в 16-ричном формате: %1").arg(command));
    }

    if (length < 1 || length > 4 || length == 3) {
        throw AssemblerException(QString("Длина команды должна быть 1, 2 или 4: %1").arg(command));
    }

    Length = length;
}

