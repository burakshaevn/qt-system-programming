#ifndef OPERAND_H
#define OPERAND_H

#include <string>
#include <variant>

class Operand
{
public:
    Operand();
    Operand(const std::string& value);
    Operand(int value);

    const std::variant<std::string, int>& getValue() const { return value_; }

    void setValue(const std::string& value) { value_ = value; }
    void setValue(int value) { value_ = value; }

    bool isString() const;
    bool isInt() const;

    std::string getStringValue() const;
    int getIntValue() const;
    
private:
    std::variant<std::string, int> value_;
};

#endif // OPERAND_H
