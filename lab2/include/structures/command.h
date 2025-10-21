#ifndef COMMAND_H
#define COMMAND_H

#include <string>

class Command
{
public:
    Command();
    Command(const std::string& name, int code, int length);

    const std::string& getName() const { return name_; }
    int getCode() const { return code_; }
    int getLength() const { return length_; }

    void setName(const std::string& name) { name_ = name; }
    void setCode(int code) { code_ = code; }
    void setLength(int length) { length_ = length; }

    bool isValid() const;
    
private:
    std::string name_;
    int code_;
    int length_;
};

#endif // COMMAND_H
