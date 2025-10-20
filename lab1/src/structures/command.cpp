#include "structures/command.h"
#include "exceptions/assemblerexception.h"
#include <regex>
#include <algorithm>

Command::Command()
    : name_(""), code_(0), length_(0)
{
}

Command::Command(const std::string& name, int code, int length)
    : name_(name), code_(code), length_(length)
{
}

bool Command::isValid() const
{
    // Check name format
    if (name_.empty()) return false;
    
    // First character must be a letter
    if (!std::isalpha(name_[0])) return false;
    
    // All characters must be alphanumeric
    for (char c : name_) {
        if (!std::isalnum(c)) return false;
    }
    
    // Check code range
    if (code_ < 0 || code_ > 255) return false;
    
    // Check length (1, 2, or 4)
    if (length_ != 1 && length_ != 2 && length_ != 4) return false;
    
    return true;
}