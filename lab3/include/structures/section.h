#ifndef SECTION_H
#define SECTION_H

#include <string>

class Section
{
public:
    Section();
    Section(const std::string& name, int startAddress = 0, int endAddress = 0, int length = 0);

    const std::string& getName() const { return name_; }
    int getStartAddress() const { return startAddress_; }
    int getEndAddress() const { return endAddress_; }
    int getLength() const { return length_; }

    void setName(const std::string& name) { name_ = name; }
    void setStartAddress(int startAddress) { startAddress_ = startAddress; }
    void setEndAddress(int endAddress) { endAddress_ = endAddress; }
    void setLength(int length) { length_ = length; }

private:
    std::string name_;
    int startAddress_;
    int endAddress_;
    int length_;
};

#endif // SECTION_H


