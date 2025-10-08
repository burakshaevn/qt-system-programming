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
    
    // Getters
    const std::variant<std::string, int>& getValue() const { return value_; }
    
    // Setters
    void setValue(const std::string& value) { value_ = value; }
    void setValue(int value) { value_ = value; }
    
    // Type checking
    bool isString() const;
    bool isInt() const;
    
    // Value extraction
    std::string getStringValue() const;
    int getIntValue() const;
    
private:
    std::variant<std::string, int> value_;
};

#endif // OPERAND_H