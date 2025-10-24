#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <vector>
#include <string>
#include <memory>
#include "structures/command.h"
#include "structures/symbolicname.h"
#include "structures/codeline.h"
#include "structures/section.h"
#include "structures/tnline.h"
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
    const std::vector<TNLine>& getTN() const { return tn_; }
    
    // Section management
    void clearSections();
    const std::vector<Section>& getSections() const { return sections_; }

    // Utility functions
    bool isCommand(const std::string& name) const;
    bool isDirective(const std::string& name) const;
    bool isLabel(const std::string& name) const;
    bool isRegister(const std::string& name) const;
    bool isCString(const std::string& str) const;
    bool isXString(const std::string& str) const;
    bool isRelativeLabel(const std::string& str) const;

    int getRegisterNumber(const std::string& reg) const;
    SymbolicName* getSymbolicName(const std::string& name, const std::string& section);
    std::string convertToASCII(const std::string& str) const;

private:
    static const int MAX_ADDRESS = 16777215; // 2^24 - 1

    std::vector<Command> availableCommands_;
    std::vector<SymbolicName> tsi_;
    std::vector<TNLine> tn_; // Modification table
    std::vector<Section> sections_;
    Section currentSection_;

    int ip_; // instruction pointer
    int secondIp_; // Second pass instruction pointer

    // Available directives
    static const std::vector<std::string> AVAILABLE_DIRECTIVES;

    // Helper functions
    void overflowCheck(int value, const std::string& textLine) const;
    void pushToTSI(const std::string& name, int address, const std::string& section, const std::string& type, const std::string& textLine);
    void pushToTN(const std::string& address, const std::string& label, const std::string& section);
    void addSection(const Section& section);
    void tsiCheck();
    void orderCheck(const std::string& directive, const std::string& previousCommand, const std::string& textLine);

    CodeLine getCodeLineFromSource(const std::vector<std::string>& line);
    CodeLine getCodeLineFromFirstPass(const std::vector<std::string>& line);

    // First pass processing
    std::string processStartDirective(const CodeLine& codeLine, const std::string& textLine, bool& startFlag);
    std::string processCsectDirective(const CodeLine& codeLine, const std::string& textLine);
    std::string processExtdefDirective(const CodeLine& codeLine, const std::string& textLine, const std::string& previousCommand);
    std::string processExtrefDirective(const CodeLine& codeLine, const std::string& textLine, const std::string& previousCommand);
    std::string processWordDirective(const CodeLine& codeLine, const std::string& textLine);
    std::string processByteDirective(const CodeLine& codeLine, const std::string& textLine);
    std::string processReswDirective(const CodeLine& codeLine, const std::string& textLine);
    std::string processResbDirective(const CodeLine& codeLine, const std::string& textLine);
    std::string processEndDirective(const CodeLine& codeLine, const std::string& textLine);

    // Second pass processing
    std::string processSecondPassExtdef(const CodeLine& codeLine, const std::string& textLine);
    std::string processSecondPassExtref(const CodeLine& codeLine, const std::string& textLine);
    std::string processSecondPassWord(const CodeLine& codeLine);
    std::string processSecondPassByte(const CodeLine& codeLine);
    std::string processSecondPassResb(const CodeLine& codeLine);
    std::string processSecondPassResw(const CodeLine& codeLine);
    std::string processSecondPassCommand(const CodeLine& codeLine, const std::string& textLine);
};

#endif // ASSEMBLER_H
