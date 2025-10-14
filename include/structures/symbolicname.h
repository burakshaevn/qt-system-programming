#ifndef SYMBOLICNAME_H
#define SYMBOLICNAME_H

#include <string>

class SymbolicName
{
public:
    SymbolicName();
    SymbolicName(const std::string& name, int address);

    const std::string& getName() const { return name_; }
    int getAddress() const { return address_; }

    void setName(const std::string& name) { name_ = name; }
    void setAddress(int address) { address_ = address; }
    
private:
    std::string name_;
    int address_;
};

#endif // SYMBOLICNAME_H
