#ifndef SYMBOLICNAME_H
#define SYMBOLICNAME_H

#include <string>

class SymbolicName
{
public:
    SymbolicName();
    SymbolicName(const std::string& name, int address, const std::string& section = "", const std::string& type = "");

    const std::string& getName() const { return name_; }
    int getAddress() const { return address_; }
    const std::string& getSection() const { return section_; }
    const std::string& getType() const { return type_; }

    void setName(const std::string& name) { name_ = name; }
    void setAddress(int address) { address_ = address; }
    void setSection(const std::string& section) { section_ = section; }
    void setType(const std::string& type) { type_ = type; }
    
private:
    std::string name_;
    int address_;
    std::string section_;
    std::string type_;  // "ВИ" (external definition), "ВС" (external reference), or "" (normal)
};

#endif // SYMBOLICNAME_H
