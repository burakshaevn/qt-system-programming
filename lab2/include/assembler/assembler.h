#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <vector>
#include <string>
#include <memory>
#include "structures/command.h"
#include "structures/symbolicname.h"
#include "structures/codeline.h"
#include "exceptions/assemblerexception.h"
#include "parser/parser.h"

class Assembler
{
public:
    Assembler();

    // Available commands management
    void setAvailableCommands(const std::vector<Command>& commands);
    const std::vector<Command>& getAvailableCommands() const { return availableCommands_; }

    // Two-pass assembly
    std::vector<std::string> firstPass(const std::vector<std::vector<std::string>>& lines, const std::string& addressingMode = "Straight");
    std::vector<std::string> secondPass(const std::vector<std::vector<std::string>>& firstPassCode);

    // Symbol table management
    void clearTSI();
    const std::vector<SymbolicName>& getTSI() const { return tsi_; }
    
    // Modification table management
    void clearTN();
    const std::vector<std::string>& getTN() const { return tn_; }

    // Utility functions
    bool isCommand(const std::string& name) const;
    bool isDirective(const std::string& name) const;
    bool isLabel(const std::string& name) const;
    bool isRegister(const std::string& name) const;
    bool isCString(const std::string& str) const;
    bool isXString(const std::string& str) const;
    bool isRelativeLabel(const std::string& str) const;

    int getRegisterNumber(const std::string& reg) const;
    SymbolicName* getSymbolicName(const std::string& name);
    std::string convertToASCII(const std::string& str) const;

private:
    static const int MAX_ADDRESS = 16777215; // 2^24 - 1

    std::vector<Command> availableCommands_;
    std::vector<SymbolicName> tsi_;
    std::vector<std::string> tn_; // Modification table

    int startAddress_;
    int endAddress_;
    int ip_; // instruction pointer
    int secondIp_; // Second pass instruction pointer

    // Available directives
    static const std::vector<std::string> AVAILABLE_DIRECTIVES;

    // Helper functions
    void overflowCheck(int value, const std::string& textLine) const;
    void pushToTSI(const std::string& name, int address);
    void pushToTN(const std::string& address);

    CodeLine getCodeLineFromSource(const std::vector<std::string>& line);
    CodeLine getCodeLineFromFirstPass(const std::vector<std::string>& line);

    // First pass processing
    std::string processStartDirective(const CodeLine& codeLine, const std::string& textLine);
    std::string processWordDirective(const CodeLine& codeLine, const std::string& textLine);
    std::string processByteDirective(const CodeLine& codeLine, const std::string& textLine);
    std::string processReswDirective(const CodeLine& codeLine, const std::string& textLine);
    std::string processResbDirective(const CodeLine& codeLine, const std::string& textLine);
    std::string processEndDirective(const CodeLine& codeLine, const std::string& textLine);

    // Second pass processing
    std::string processSecondPassWord(const CodeLine& codeLine);
    std::string processSecondPassByte(const CodeLine& codeLine);
    std::string processSecondPassResb(const CodeLine& codeLine);
    std::string processSecondPassResw(const CodeLine& codeLine);
    std::string processSecondPassCommand(const CodeLine& codeLine);
};

#endif // ASSEMBLER_H
