#ifndef ASSEMBLEREXCEPTION_H
#define ASSEMBLEREXCEPTION_H

#include <QString>
#include <exception>

class AssemblerException : public std::exception
{
private:
    QString message;

public:
    AssemblerException();
    AssemblerException(const QString& msg);
    
    const char* what() const noexcept override;
    QString getMessage() const { return message; }
};

#endif // ASSEMBLEREXCEPTION_H

