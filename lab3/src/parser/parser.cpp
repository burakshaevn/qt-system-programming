#include "parser/parser.h"
#include <regex>
#include <sstream>
#include <algorithm>
#include <cctype>

std::vector<std::vector<std::string>> Parser::parseCode(const std::string& input)
{
    std::vector<std::vector<std::string>> result;
    std::istringstream stream(input);
    std::string line;

    while (std::getline(stream, line)) {
        // Replace tabs with spaces
        std::replace(line.begin(), line.end(), '\t', ' ');

        // Tokenize the line
        std::vector<std::string> tokens = tokenizeLine(line);

        // Filter out empty tokens
        tokens.erase(std::remove_if(tokens.begin(), tokens.end(),
                                    [](const std::string& token) { return token.empty(); }), tokens.end());

        if (!tokens.empty()) {
            result.push_back(tokens);
        }
    }

    return result;
}

std::vector<std::string> Parser::tokenizeLine(const std::string& line)
{
    std::vector<std::string> tokens;
    std::regex pattern(R"((?:[CX]"[^"]*(?:"[^"]*)*"|\S+))");
    std::sregex_iterator iter(line.begin(), line.end(), pattern);
    std::sregex_iterator end;

    for (; iter != end; ++iter) {
        tokens.push_back(iter->str());
    }

    return tokens;
}

std::vector<Command> Parser::textToCommands(const std::string& text)
{
    std::vector<std::vector<std::string>> lines = parseCode(text);
    std::vector<Command> commands;

    for (const auto& line : lines) {
        if (line.size() != 3) {
            std::string lineStr;
            for (const auto& token : line) {
                lineStr += token + " ";
            }
            throw AssemblerException("Неправильный формат строки: " + lineStr);
        }

        try {
            std::string name = line[0];
            int code = std::stoi(line[1], nullptr, 16);
            int length = std::stoi(line[2], nullptr, 16);

            Command cmd(name, code, length);
            if (!cmd.isValid()) {
                std::string lineStr;
                for (const auto& token : line) {
                    lineStr += token + " ";
                }
                throw AssemblerException("Недопустимая команда: " + lineStr);
            }

            commands.push_back(cmd);
        } catch (const std::exception& e) {
            std::string lineStr;
            for (const auto& token : line) {
                lineStr += token + " ";
            }
            throw AssemblerException("Ошибка парсинга команды: " + lineStr);
        }
    }

    return commands;
}

bool Parser::isRegister(const std::string& token) {
    std::regex regPattern(R"(^R(?:[1-9]|1[0-6])$)");
    return std::regex_match(token, regPattern);
}

CodeLine Parser::parseCodeLine(const std::vector<std::string>& line)
{
    if (line.empty() || line.size() > 4) {
        std::string lineStr;
        for (const auto& token : line) {
            lineStr += token + " ";
        }
        throw AssemblerException("Неверный формат команды: " + lineStr);
    }

    CodeLine codeLine;

    switch (line.size()) {
    case 1:
        // Only command (operandless command or END)
        codeLine.setCommand(line[0]);
        break;

    case 2:
        // Could be label + command or command + operand
        // If second token looks like a command/directive, then first is label
        if (isCommandOrDirective(line[1])) {
            codeLine.setLabel(line[0]);
            codeLine.setCommand(line[1]);
        } else {
            codeLine.setCommand(line[0]);
            codeLine.setFirstOperand(line[1]);
        }
        break;

    case 3:
        if (isRegister(line[0])) {
            throw AssemblerException("Регистр не может использоваться как метка: " + line[0]);
        }
        // Could be label + command + operand or command + operand1 + operand2
        // If second token looks like a command/directive, then first is label
        if (isCommandOrDirective(line[1])) {
            codeLine.setLabel(line[0]);
            codeLine.setCommand(line[1]);
            codeLine.setFirstOperand(line[2]);
        } else {
            codeLine.setCommand(line[0]);
            codeLine.setFirstOperand(line[1]);
            codeLine.setSecondOperand(line[2]);
        }
        break;

    case 4:
        // label + command + operand1 + operand2
        codeLine.setLabel(line[0]);
        codeLine.setCommand(line[1]);
        codeLine.setFirstOperand(line[2]);
        codeLine.setSecondOperand(line[3]);
        break;
    }

    return codeLine;
}

CodeLine Parser::parseFirstPassLine(const std::vector<std::string>& line)
{
    if (line.size() < 2 || line.size() > 4) {
        std::string lineStr;
        for (const auto& token : line) {
            lineStr += token + " ";
        }
        throw AssemblerException("Неверный формат команды первого прохода: " + lineStr);
    }

    CodeLine codeLine;

    switch (line.size()) {
    case 2:
        {
            // Special case for EXTDEF/EXTREF: command + operand (no address)
            std::string upperFirst = line[0];
            std::transform(upperFirst.begin(), upperFirst.end(), upperFirst.begin(), ::toupper);
            
            if (upperFirst == "EXTDEF" || upperFirst == "EXTREF") {
                // EXTDEF/EXTREF + Label
                codeLine.setCommand(line[0]);
                codeLine.setFirstOperand(line[1]);
            } else {
                // address + command
                codeLine.setLabel(line[0]);
                codeLine.setCommand(line[1]);
            }
        }
        break;

    case 3:
        // address + command + operand
        codeLine.setLabel(line[0]);
        codeLine.setCommand(line[1]);
        codeLine.setFirstOperand(line[2]);
        break;

    case 4:
        // address + command + operand1 + operand2
        codeLine.setLabel(line[0]);
        codeLine.setCommand(line[1]);
        codeLine.setFirstOperand(line[2]);
        codeLine.setSecondOperand(line[3]);
        break;
    }

    return codeLine;
}

bool Parser::isCommandOrDirective(const std::string& token)
{
    // Known directives
    static const std::vector<std::string> directives = {
        "START", "END", "WORD", "BYTE", "RESB", "RESW", "EXTDEF", "EXTREF", "CSECT"
    };

    // Known commands (default set)
    static const std::vector<std::string> commands = {
        "JMP", "LOADR1", "LOADR2", "ADD", "SAVER1", "INT"
    };

    std::string upperToken = token;
    std::transform(upperToken.begin(), upperToken.end(), upperToken.begin(), ::toupper);

    // Check if it's a directive
    for (const auto& dir : directives) {
        if (upperToken == dir) return true;
    }

    // Check if it's a command
    for (const auto& cmd : commands) {
        if (upperToken == cmd) return true;
    }

    // Don't use heuristics - be strict about what's a command/directive
    // This prevents labels like "BUFFER" from being mistaken as commands
    return false;
}

bool Parser::isValidCommandFormat(const std::vector<std::string>& line)
{
    if (line.size() < 2) return false;

    // Check if first token looks like a command (not a label)
    const std::string& first = line[0];

    // Commands are usually uppercase and short
    if (first.length() > 10) return false;

    // Check if it starts with a letter and contains only alphanumeric characters
    if (!std::isalpha(first[0])) return false;

    for (char c : first) {
        if (!std::isalnum(c)) return false;
    }

    return true;
}
