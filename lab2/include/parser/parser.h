#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <string>
#include "structures/codeline.h"
#include "structures/command.h"
#include "exceptions/assemblerexception.h"

class Parser
{
public:
    // Parse source code into lines of tokens
    static std::vector<std::vector<std::string>> parseCode(const std::string& input);

    // Parse command definitions from text
    static std::vector<Command> textToCommands(const std::string& text);

    // Parse a single line into CodeLine
    static CodeLine parseCodeLine(const std::vector<std::string>& line);

    // Parse first pass result line
    static CodeLine parseFirstPassLine(const std::vector<std::string>& line);

private:
    // Helper functions
    static std::vector<std::string> tokenizeLine(const std::string& line);
    static bool isValidCommandFormat(const std::vector<std::string>& line);
    static bool isCommandOrDirective(const std::string& token);
    static bool isRegister(const std::string& token);
};

#endif // PARSER_H
