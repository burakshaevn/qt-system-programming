#ifndef TNLINE_H
#define TNLINE_H

#include <string>

class TNLine
{
public:
    TNLine();
    TNLine(const std::string& address, const std::string& label, const std::string& section);

    const std::string& getAddress() const { return address_; }
    const std::string& getLabel() const { return label_; }
    const std::string& getSection() const { return section_; }

    void setAddress(const std::string& address) { address_ = address; }
    void setLabel(const std::string& label) { label_ = label; }
    void setSection(const std::string& section) { section_ = section; }

private:
    std::string address_;
    std::string label_;
    std::string section_;
};

#endif // TNLINE_H


