#ifndef CODELINE_H
#define CODELINE_H

#include <string>

class CodeLine
{
public:
    CodeLine();
    CodeLine(const std::string& command);
    CodeLine(const std::string& command, const std::string& firstOperand);
    CodeLine(const std::string& command, const std::string& firstOperand, const std::string& secondOperand);
    CodeLine(const std::string& label, const std::string& command, const std::string& firstOperand, const std::string& secondOperand);

    const std::string& getLabel() const { return label_; }
    const std::string& getCommand() const { return command_; }
    const std::string& getFirstOperand() const { return firstOperand_; }
    const std::string& getSecondOperand() const { return secondOperand_; }

    void setLabel(const std::string& label) { label_ = label; }
    void setCommand(const std::string& command) { command_ = command; }
    void setFirstOperand(const std::string& operand) { firstOperand_ = operand; }
    void setSecondOperand(const std::string& operand) { secondOperand_ = operand; }

    bool hasLabel() const { return !label_.empty(); }
    bool hasFirstOperand() const { return !firstOperand_.empty(); }
    bool hasSecondOperand() const { return !secondOperand_.empty(); }
    
private:
    std::string label_;
    std::string command_;
    std::string firstOperand_;
    std::string secondOperand_;
};

#endif // CODELINE_H
