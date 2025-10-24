#include "assembler/assembler.h"
#include <algorithm>
#include <regex>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <set>

const std::vector<std::string> Assembler::AVAILABLE_DIRECTIVES = {
    "START", "END", "WORD", "BYTE", "RESB", "RESW", "EXTDEF", "EXTREF", "CSECT"
};

Assembler::Assembler()
    : ip_(0), secondIp_(0)
{
    // Initialize with default commands
    availableCommands_ = {
        Command("JMP", 1, 4),
        Command("LOADR1", 2, 4),
        Command("LOADR2", 3, 4),
        Command("ADD", 4, 2),
        Command("SAVER1", 5, 4),
        Command("INT", 6, 2)
    };
}

void Assembler::setAvailableCommands(const std::vector<Command>& commands)
{
    // Check name uniqueness
    std::set<std::string> names;
    for (const auto& cmd : commands) {
        std::string upperName = cmd.getName();
        std::transform(upperName.begin(), upperName.end(), upperName.begin(), ::toupper);
        if (!names.insert(upperName).second) {
            throw AssemblerException("Все имена команд должны быть уникальными");
        }
    }

    // Check code uniqueness
    std::set<int> codes;
    for (const auto& cmd : commands) {
        if (!codes.insert(cmd.getCode()).second) {
            throw AssemblerException("Все коды команд должны быть уникальными");
        }
    }

    availableCommands_ = commands;
}

void Assembler::clearTSI()
{
    tsi_.clear();
}

void Assembler::clearTN()
{
    tn_.clear();
}

void Assembler::clearSections()
{
    sections_.clear();
}

void Assembler::pushToTN(const std::string& address, const std::string& label, const std::string& section)
{
    tn_.emplace_back(address, label, section);
}

void Assembler::addSection(const Section& section)
{
    // Check if section name is unique
    for (const auto& s : sections_) {
        std::string upperSectionName = s.getName();
        std::string upperNewName = section.getName();
        std::transform(upperSectionName.begin(), upperSectionName.end(), upperSectionName.begin(), ::toupper);
        std::transform(upperNewName.begin(), upperNewName.end(), upperNewName.begin(), ::toupper);
        
        if (upperSectionName == upperNewName) {
            throw AssemblerException("Все имена секций должны быть уникальными: " + section.getName());
        }
    }
    
    // Check if total length would overflow
    int totalLength = section.getLength();
    for (const auto& s : sections_) {
        totalLength += s.getLength();
    }
    overflowCheck(totalLength, section.getName());
    
    sections_.push_back(section);
}

void Assembler::tsiCheck()
{
    for (const auto& sym : tsi_) {
        if (sym.getType() == "ВИ" && sym.getAddress() == -1) {
            throw AssemblerException("Не всем внешним именам было присвоено значение");
        }
    }
}

void Assembler::orderCheck(const std::string& directive, const std::string& previousCommand, const std::string& textLine)
{
    std::string upperDirective = directive;
    std::string upperPrevious = previousCommand;
    std::transform(upperDirective.begin(), upperDirective.end(), upperDirective.begin(), ::toupper);
    std::transform(upperPrevious.begin(), upperPrevious.end(), upperPrevious.begin(), ::toupper);
    
    if (upperDirective == "EXTDEF") {
        if (upperPrevious != "START" && upperPrevious != "CSECT" && upperPrevious != "EXTDEF") {
            throw AssemblerException("Директива EXTDEF может стоять только после директив START, CSECT и EXTDEF: " + textLine);
        }
    } else if (upperDirective == "EXTREF") {
        if (upperPrevious != "START" && upperPrevious != "CSECT" && upperPrevious != "EXTDEF" && upperPrevious != "EXTREF") {
            throw AssemblerException("Директива EXTREF может стоять только после директив START, CSECT, EXTDEF и EXTREF: " + textLine);
        }
    }
}

bool Assembler::isCommand(const std::string& name) const
{
    if (name.empty()) return false;

    std::string upperName = name;
    std::transform(upperName.begin(), upperName.end(), upperName.begin(), ::toupper);

    return std::any_of(availableCommands_.begin(), availableCommands_.end(),
                       [&upperName](const Command& cmd) {
                           std::string cmdName = cmd.getName();
                           std::transform(cmdName.begin(), cmdName.end(), cmdName.begin(), ::toupper);
                           return cmdName == upperName;
                       });
}

bool Assembler::isDirective(const std::string& name) const
{
    if (name.empty()) return false;

    std::string upperName = name;
    std::transform(upperName.begin(), upperName.end(), upperName.begin(), ::toupper);

    return std::find(AVAILABLE_DIRECTIVES.begin(), AVAILABLE_DIRECTIVES.end(), upperName) != AVAILABLE_DIRECTIVES.end();
}

bool Assembler::isLabel(const std::string& name) const
{
    if (isRegister(name)) {
        return false; // Регистр не может быть меткой
    }
    if (name.empty() || name.length() > 10) return false;

    // Must start with a letter
    if (!std::isalpha(name[0])) return false;

    // Must contain only alphanumeric characters and underscore
    for (char c : name) {
        if (!std::isalnum(c) && c != '_') return false;
    }

    // Must not be a register
    if (isRegister(name)) return false;

    // Must not be a command or directive
    if (isCommand(name) || isDirective(name)) return false;

    return true;
}

bool Assembler::isRegister(const std::string& name) const
{
    if (name.empty()) return false;

    std::regex regPattern(R"(^R(?:[1-9]|1[0-6])$)");
    return std::regex_match(name, regPattern);
}

bool Assembler::isRelativeLabel(const std::string& str) const
{
    if (str.empty() || str.length() < 3) return false;

    // Must start with [ and end with ]
    if (str.front() != '[' || str.back() != ']') return false;

    // Extract the label inside brackets
    std::string labelInside = str.substr(1, str.length() - 2);

    // Check if it's a valid label
    return isLabel(labelInside);
}

bool Assembler::isCString(const std::string& str) const
{
    if (str.length() < 4) return false;

    // Must start with C" and end with "
    if (str[0] != 'C' || str[1] != '"' || str.back() != '"') return false;

    // Check if all characters are printable ASCII
    std::string content = str.substr(2, str.length() - 3);
    for (char c : content) {
        if (c < 32 || c > 126) return false; // Printable ASCII range
    }

    return true;
}

bool Assembler::isXString(const std::string& str) const
{
    if (str.length() < 4) return false;

    // Must start with X" and end with "
    if (str[0] != 'X' || str[1] != '"' || str.back() != '"') return false;

    // Check if all characters are valid hex digits
    std::string content = str.substr(2, str.length() - 3);
    if (content.empty() || content.length() % 2 != 0) return false;

    for (char c : content) {
        if (!std::isxdigit(c)) return false;
    }

    return true;
}

int Assembler::getRegisterNumber(const std::string& reg) const
{
    if (!isRegister(reg)) {
        throw AssemblerException("Invalid register: " + reg);
    }

    return std::stoi(reg.substr(1));
}

SymbolicName* Assembler::getSymbolicName(const std::string& name, const std::string& section)
{
    std::string upperName = name;
    std::transform(upperName.begin(), upperName.end(), upperName.begin(), ::toupper);

    auto it = std::find_if(tsi_.begin(), tsi_.end(),
                           [&upperName, &section](const SymbolicName& sym) {
                               std::string symName = sym.getName();
                               std::transform(symName.begin(), symName.end(), symName.begin(), ::toupper);
                               return symName == upperName && sym.getSection() == section;
                           });

    return (it != tsi_.end()) ? &(*it) : nullptr;
}

std::string Assembler::convertToASCII(const std::string& str) const
{
    std::stringstream result;
    for (char c : str) {
        result << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << (int)(unsigned char)c;
    }
    return result.str();
}

void Assembler::overflowCheck(int value, const std::string& textLine) const
{
    if (value < 0 || value > MAX_ADDRESS) {
        throw AssemblerException("Выход за границы выделенной памяти: " + textLine);
    }
}

void Assembler::pushToTSI(const std::string& name, int address, const std::string& section, const std::string& type, const std::string& textLine)
{
    std::string upperName = name;
    std::transform(upperName.begin(), upperName.end(), upperName.begin(), ::toupper);
    
    // Check if symbol already exists in the same section
    for (auto& sym : tsi_) {
        std::string upperSymName = sym.getName();
        std::transform(upperSymName.begin(), upperSymName.end(), upperSymName.begin(), ::toupper);
        
        if (sym.getSection() == section && upperSymName == upperName) {
            // If it's an external definition (ВИ) and we're trying to set its address
            if (sym.getType() == "ВИ") {
                if (type == "ВИ" || type == "ВС") {
                    throw AssemblerException("Такая метка уже есть в ТСИ: " + textLine);
                } else {
                    // Setting address for EXTDEF
                    if (sym.getAddress() == -1 && address != -1) {
                        sym.setAddress(address);
                        return;
                    } else {
                        throw AssemblerException("Такая метка уже есть в ТСИ: " + textLine);
                    }
                }
            } else {
                throw AssemblerException("Такая метка уже есть в ТСИ: " + textLine);
            }
        }
    }
    
    tsi_.emplace_back(upperName, address, section, type);
}

std::vector<std::string> Assembler::firstPass(const std::vector<std::vector<std::string>>& lines, const std::string& addressingMode)
{
    std::vector<std::string> firstPassCode;
    std::string previousCommand = "";

    ip_ = 0;

    bool startFlag = false;
    bool endFlag = false;

    for (const auto& line : lines) {
        std::string textLine;
        for (const auto& token : line) {
            textLine += token + " ";
        }

        std::string firstPassLine;

        if (!startFlag && ip_ != 0) {
            throw AssemblerException("Не найдена директива START в начале программы");
        }

        if (startFlag) {
            overflowCheck(ip_, textLine);
        }

        if (endFlag) break;

        CodeLine codeLine = getCodeLineFromSource(line);
        
        if (codeLine.getCommand().empty()) {
            throw AssemblerException("Пустая команда в строке: " + textLine);
        }
        
        std::string upperCmd = codeLine.getCommand();
        std::transform(upperCmd.begin(), upperCmd.end(), upperCmd.begin(), ::toupper);
        
        // Debug: Check what we're processing
        bool isDir = isDirective(upperCmd);
        bool isCmd = isCommand(upperCmd);
        
        if (!isDir && !isCmd) {
            std::string debugInfo = "Команда: '" + upperCmd + "', Оригинал: '" + codeLine.getCommand() + 
                                   "', isDirective: " + (isDir ? "true" : "false") + 
                                   ", isCommand: " + (isCmd ? "true" : "false");
            throw AssemblerException("Неизвестная команда или директива. " + debugInfo + ". Строка: " + textLine);
        }

        // Process command part (use upperCmd for consistency)
        if (isDirective(upperCmd)) {
            if (upperCmd == "START") {
                firstPassLine = processStartDirective(codeLine, textLine, startFlag);
            } else if (upperCmd == "CSECT") {
                firstPassLine = processCsectDirective(codeLine, textLine);
            } else if (upperCmd == "EXTDEF") {
                firstPassLine = processExtdefDirective(codeLine, textLine, previousCommand);
            } else if (upperCmd == "EXTREF") {
                firstPassLine = processExtrefDirective(codeLine, textLine, previousCommand);
            } else if (upperCmd == "WORD") {
                if (codeLine.hasLabel()) {
                    pushToTSI(codeLine.getLabel(), ip_, currentSection_.getName(), "", textLine);
                }
                firstPassLine = processWordDirective(codeLine, textLine);
            } else if (upperCmd == "BYTE") {
                if (codeLine.hasLabel()) {
                    pushToTSI(codeLine.getLabel(), ip_, currentSection_.getName(), "", textLine);
                }
                firstPassLine = processByteDirective(codeLine, textLine);
            } else if (upperCmd == "RESW") {
                if (codeLine.hasLabel()) {
                    pushToTSI(codeLine.getLabel(), ip_, currentSection_.getName(), "", textLine);
                }
                firstPassLine = processReswDirective(codeLine, textLine);
            } else if (upperCmd == "RESB") {
                if (codeLine.hasLabel()) {
                    pushToTSI(codeLine.getLabel(), ip_, currentSection_.getName(), "", textLine);
                }
                firstPassLine = processResbDirective(codeLine, textLine);
            } else if (upperCmd == "END") {
                if (!startFlag || endFlag) {
                    throw AssemblerException("Не найдена метка START либо ошибка в директивах START/END: " + textLine);
                }
                
                if (codeLine.hasLabel()) {
                    pushToTSI(codeLine.getLabel(), ip_, currentSection_.getName(), "", textLine);
                }
                
                processEndDirective(codeLine, textLine);
                endFlag = true;
                continue;
            }
        } else if (isCommand(upperCmd)) {
            if (codeLine.hasLabel()) {
                pushToTSI(codeLine.getLabel(), ip_, currentSection_.getName(), "", textLine);
            }
            // Find the command
            auto cmdIt = std::find_if(availableCommands_.begin(), availableCommands_.end(),
                                      [&codeLine](const Command& cmd) {
                                          std::string cmdName = cmd.getName();
                                          std::transform(cmdName.begin(), cmdName.end(), cmdName.begin(), ::toupper);
                                          std::string lineCmd = codeLine.getCommand();
                                          std::transform(lineCmd.begin(), lineCmd.end(), lineCmd.begin(), ::toupper);
                                          return cmdName == lineCmd;
                                      });

            if (cmdIt == availableCommands_.end()) {
                throw AssemblerException("Неизвестная команда: " + textLine);
            }

            const Command& command = *cmdIt;

            switch (command.getLength()) {
            case 1: {
                if (codeLine.hasFirstOperand()) {
                    throw AssemblerException("Ожидается ноль операндов: " + textLine);
                }
                overflowCheck(ip_ + 1, textLine);
                std::stringstream ss;
                ss << std::hex << std::uppercase << std::setfill('0') << std::setw(6) << ip_
                   << " " << std::setw(2) << (command.getCode() * 4 + 0);
                firstPassLine = ss.str();
                ip_ += 1;
                break;
            }
            case 2:
                if (!codeLine.hasFirstOperand()) {
                    throw AssemblerException("Ожидается минимум один операнд, но было получено ноль: " + textLine);
                }

                if (codeLine.hasSecondOperand()) {
                    // Two registers
                    if (isRegister(codeLine.getFirstOperand()) && isRegister(codeLine.getSecondOperand())) {
                        overflowCheck(ip_ + 2, textLine);
                        std::stringstream ss;
                        ss << std::hex << std::uppercase << std::setfill('0') << std::setw(6) << ip_
                           << " " << std::setw(2) << (command.getCode() * 4 + 0)
                           << " " << codeLine.getFirstOperand() << " " << codeLine.getSecondOperand();
                        firstPassLine = ss.str();
                        ip_ += 2;
                    } else {
                        throw AssemblerException("Неверный формат команды. Ожидалось два регистра: " + textLine);
                    }
                } else {
                    // One byte value
                    try {
                        int value = std::stoi(codeLine.getFirstOperand());
                        if (value < 0 || value > 255) {
                            throw AssemblerException("Значение первого операнда выходит за границы допустимого диапазона (0-255): " + textLine);
                        }
                        overflowCheck(ip_ + 2, textLine);
                        std::stringstream ss;
                        ss << std::hex << std::uppercase << std::setfill('0') << std::setw(6) << ip_
                           << " " << std::setw(2) << (command.getCode() * 4 + 0)
                           << " " << std::setw(2) << value;
                        firstPassLine = ss.str();
                        ip_ += 2;
                    } catch (const std::exception&) {
                        throw AssemblerException("Невозможно преобразовать первый операнд в число: " + textLine);
                    }
                }
                break;

            case 4:
                if (!codeLine.hasFirstOperand()) {
                    throw AssemblerException("Ожидается один операнд, но было получено ноль: " + textLine);
                }
                if (codeLine.hasSecondOperand()) {
                    throw AssemblerException("Ожидается один операнд, но найдено два: " + textLine);
                }

                // Check for relative addressing [LABEL]
                if (isRelativeLabel(codeLine.getFirstOperand())) {
                    if (addressingMode == "Straight") {
                        throw AssemblerException("Данный тип адресации недоступен в этом режиме адресации: " + textLine);
                    }
                    
                    overflowCheck(ip_ + 4, textLine);
                    std::stringstream ss;
                    ss << std::hex << std::uppercase << std::setfill('0') << std::setw(6) << ip_
                       << " " << std::setw(2) << (command.getCode() * 4 + 2)
                       << " " << codeLine.getFirstOperand();
                    firstPassLine = ss.str();
                    ip_ += 4;
                } else if (isLabel(codeLine.getFirstOperand())) {
                    // Direct addressing with label
                    if (addressingMode == "Relative") {
                        throw AssemblerException("Данный тип адресации недоступен в этом режиме адресации: " + textLine);
                    }
                    
                    overflowCheck(ip_ + 4, textLine);
                    std::stringstream ss;
                    ss << std::hex << std::uppercase << std::setfill('0') << std::setw(6) << ip_
                       << " " << std::setw(2) << (command.getCode() * 4 + 1)
                       << " " << codeLine.getFirstOperand();
                    firstPassLine = ss.str();
                    ip_ += 4;
                } else {
                    try {
                        int value = std::stoi(codeLine.getFirstOperand());
                        if (value < 0 || value > 16777215) {
                            throw AssemblerException("Недопустимое значение операнда: " + textLine);
                        }
                        overflowCheck(ip_ + 4, textLine);
                        std::stringstream ss;
                        ss << std::hex << std::uppercase << std::setfill('0') << std::setw(6) << ip_
                           << " " << std::setw(2) << (command.getCode() * 4)
                           << " " << std::setw(6) << value;
                        firstPassLine = ss.str();
                        ip_ += 4;
                    } catch (const std::exception&) {
                        throw AssemblerException("Недопустимое значение операнда: " + textLine);
                    }
                }
                break;
            }
        }
        // If we reach here, the command was processed successfully

        previousCommand = codeLine.getCommand();
        firstPassCode.push_back(firstPassLine);
    }

    if (!endFlag) {
        throw AssemblerException("Не найдена точка входа в программу.");
    }
    
    tsiCheck();

    return firstPassCode;
}

std::string Assembler::processStartDirective(const CodeLine& codeLine, const std::string& textLine, bool& startFlag)
{
    if (codeLine.hasSecondOperand()) {
        throw AssemblerException("Ожидается один операнд, но найдено два: " + textLine);
    }

    if (ip_ != 0 || startFlag) {
        throw AssemblerException("START должен быть единственным, в начале исходного кода: " + textLine);
    }

    if (!codeLine.hasLabel()) {
        throw AssemblerException("Перед директивой START должна быть метка: " + textLine);
    }

    int address = 0;
    if (codeLine.hasFirstOperand()) {
        try {
            address = std::stoi(codeLine.getFirstOperand());
        } catch (const std::exception&) {
            throw AssemblerException("Невозможно преобразовать первый операнд в адрес начала программы: " + textLine);
        }

        if (address != 0) {
            throw AssemblerException("Адрес загрузки должен быть равен нулю: " + textLine);
        }
    }

    overflowCheck(address, textLine);

    startFlag = true;
    
    // Initialize currentSection
    currentSection_.setName(codeLine.getLabel());
    currentSection_.setStartAddress(address);

    ip_ = address;

    std::stringstream ss;
    ss << codeLine.getLabel() << "\t" << codeLine.getCommand() << "\t" 
       << std::hex << std::uppercase << std::setfill('0') << std::setw(6) << address;
    return ss.str();
}

std::string Assembler::processCsectDirective(const CodeLine& codeLine, const std::string& textLine)
{
    if (codeLine.hasSecondOperand()) {
        throw AssemblerException("Ожидается ноль или один операнд: " + textLine);
    }

    if (!codeLine.hasLabel()) {
        throw AssemblerException("Перед директивой CSECT должна быть метка: " + textLine);
    }

    int endAddress = 0;
    if (codeLine.hasFirstOperand()) {
        try {
            endAddress = std::stoi(codeLine.getFirstOperand());
        } catch (const std::exception&) {
            throw AssemblerException("Невозможно преобразовать первый операнд в адрес входа в секцию: " + textLine);
        }

        if (endAddress < 0 || endAddress > 16777215) {
            throw AssemblerException("Значение первого операнда выходит за границы допустимого диапазона (0-16777215): " + textLine);
        }
    }

    // Update and save previous section
    currentSection_.setEndAddress(endAddress);
    currentSection_.setLength(ip_ - currentSection_.getStartAddress());
    addSection(currentSection_);

    // Initialize new section
    currentSection_.setName(codeLine.getLabel());
    currentSection_.setStartAddress(0);
    currentSection_.setEndAddress(0);
    currentSection_.setLength(0);

    ip_ = 0;

    std::stringstream ss;
    ss << codeLine.getLabel() << "\t" << codeLine.getCommand() << "\t" 
       << std::hex << std::uppercase << std::setfill('0') << std::setw(6) << endAddress;
    return ss.str();
}

std::string Assembler::processExtdefDirective(const CodeLine& codeLine, const std::string& textLine, const std::string& previousCommand)
{
    if (!codeLine.hasFirstOperand()) {
        throw AssemblerException("Ожидается один операнд, но было получено ноль: " + textLine);
    }
    if (codeLine.hasSecondOperand()) {
        throw AssemblerException("Ожидается один операнд, но найдено два: " + textLine);
    }

    orderCheck("EXTDEF", previousCommand, textLine);

    if (!isLabel(codeLine.getFirstOperand())) {
        throw AssemblerException("Операнд для директивы EXTDEF должен быть меткой: " + textLine);
    }

    pushToTSI(codeLine.getFirstOperand(), -1, currentSection_.getName(), "ВИ", textLine);

    std::stringstream ss;
    ss << "\t" << "EXTDEF" << "\t" << codeLine.getFirstOperand();
    return ss.str();
}

std::string Assembler::processExtrefDirective(const CodeLine& codeLine, const std::string& textLine, const std::string& previousCommand)
{
    if (!codeLine.hasFirstOperand()) {
        throw AssemblerException("Ожидается один операнд, но было получено ноль: " + textLine);
    }
    if (codeLine.hasSecondOperand()) {
        throw AssemblerException("Ожидается один операнд, но найдено два: " + textLine);
    }

    orderCheck("EXTREF", previousCommand, textLine);

    if (!isLabel(codeLine.getFirstOperand())) {
        throw AssemblerException("Операнд для директивы EXTREF должен быть меткой: " + textLine);
    }

    pushToTSI(codeLine.getFirstOperand(), -1, currentSection_.getName(), "ВС", textLine);

    std::stringstream ss;
    ss << "\t" << "EXTREF" << "\t" << codeLine.getFirstOperand();
    return ss.str();
}

std::string Assembler::processWordDirective(const CodeLine& codeLine, const std::string& textLine)
{
    if (!codeLine.hasFirstOperand()) {
        throw AssemblerException("Ожидается один операнд, но было получено ноль: " + textLine);
    }

    if (codeLine.hasSecondOperand()) {
        throw AssemblerException("Ожидается один операнд, но найдено два: " + textLine);
    }

    int value;
    try {
        value = std::stoi(codeLine.getFirstOperand());
    } catch (const std::exception&) {
        throw AssemblerException("Невозможно преобразовать первый операнд в число: " + textLine);
    }

    if (value <= 0 || value > 16777215) {
        throw AssemblerException("Значение первого операнда выходит за границы допустимого диапазона (1-16777215): " + textLine);
    }

    overflowCheck(ip_ + 3, textLine);

    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0') << std::setw(6) << ip_ << " WORD " << std::setw(6) << value;
    ip_ += 3;
    return ss.str();
}

std::string Assembler::processByteDirective(const CodeLine& codeLine, const std::string& textLine)
{
    if (!codeLine.hasFirstOperand()) {
        throw AssemblerException("Ожидается один операнд, но было получено ноль: " + textLine);
    }

    if (codeLine.hasSecondOperand()) {
        throw AssemblerException("Ожидается один операнд, но найдено два: " + textLine);
    }

    const std::string& operand = codeLine.getFirstOperand();

    // Try to parse as numeric value
    try {
        int value = std::stoi(operand);
        if (value < 0 || value > 255) {
            throw AssemblerException("Значение первого операнда выходит за границы допустимого диапазона (0-255): " + textLine);
        }

        overflowCheck(ip_ + 1, textLine);

        std::stringstream ss;
        ss << std::hex << std::uppercase << std::setfill('0') << std::setw(6) << ip_ << " BYTE " << std::setw(2) << value;
        ip_ += 1;
        return ss.str();
    } catch (const std::exception&) {
        // Try to parse as string
        if (isCString(operand)) {
            std::string symbols = operand.substr(2, operand.length() - 3);
            overflowCheck(ip_ + symbols.length(), textLine);

            std::stringstream ss;
            ss << std::hex << std::uppercase << std::setfill('0') << std::setw(6) << ip_ << " BYTE " << operand;
            ip_ += symbols.length();
            return ss.str();
        } else if (isXString(operand)) {
            std::string symbols = operand.substr(2, operand.length() - 3);
            overflowCheck(ip_ + symbols.length() / 2, textLine);

            std::stringstream ss;
            ss << std::hex << std::uppercase << std::setfill('0') << std::setw(6) << ip_ << " BYTE " << operand;
            ip_ += symbols.length() / 2;
            return ss.str();
        } else {
            throw AssemblerException("Невозможно преобразовать первый операнд в символьную или шестнадцатеричную строку: " + textLine);
        }
    }
}

std::string Assembler::processReswDirective(const CodeLine& codeLine, const std::string& textLine)
{
    if (!codeLine.hasFirstOperand()) {
        throw AssemblerException("Ожидается один операнд, но было получено ноль: " + textLine);
    }

    if (codeLine.hasSecondOperand()) {
        throw AssemblerException("Ожидается один операнд, но найдено два: " + textLine);
    }

    int value;
    try {
        value = std::stoi(codeLine.getFirstOperand());
    } catch (const std::exception&) {
        throw AssemblerException("Невозможно преобразовать первый операнд в число: " + textLine);
    }

    if (value <= 0 || value > 255) {
        throw AssemblerException("Значение первого операнда выходит за границы допустимого диапазона (1-255): " + textLine);
    }

    overflowCheck(ip_ + value * 3, textLine);

    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0') << std::setw(6) << ip_ << " RESW " << std::setw(2) << value;
    ip_ += value * 3;
    return ss.str();
}

std::string Assembler::processResbDirective(const CodeLine& codeLine, const std::string& textLine)
{
    if (!codeLine.hasFirstOperand()) {
        throw AssemblerException("Ожидается один операнд, но было получено ноль: " + textLine);
    }

    if (codeLine.hasSecondOperand()) {
        throw AssemblerException("Ожидается один операнд, но найдено два: " + textLine);
    }

    int value;
    try {
        value = std::stoi(codeLine.getFirstOperand());
    } catch (const std::exception&) {
        throw AssemblerException("Невозможно преобразовать первый операнд в число: " + textLine);
    }

    if (value <= 0 || value > 255) {
        throw AssemblerException("Значение первого операнда выходит за границы допустимого диапазона (1-255): " + textLine);
    }

    overflowCheck(ip_ + value, textLine);

    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0') << std::setw(6) << ip_ << " RESB " << std::setw(2) << value;
    ip_ += value;
    return ss.str();
}

std::string Assembler::processEndDirective(const CodeLine& codeLine, const std::string& textLine)
{
    if (codeLine.hasSecondOperand()) {
        throw AssemblerException("Ожидается максимум один операнд, но найдено два: " + textLine);
    }

    int endAddress = 0;
    if (codeLine.hasFirstOperand()) {
        try {
            endAddress = std::stoi(codeLine.getFirstOperand());
        } catch (const std::exception&) {
            throw AssemblerException("Невозможно преобразовать первый операнд в адрес входа в программу: " + textLine);
        }

        if (endAddress < 0 || endAddress > 16777215) {
            throw AssemblerException("Значение первого операнда выходит за границы допустимого диапазона (0-16777215): " + textLine);
        }

        overflowCheck(endAddress, textLine);
    }

    // Update and save current section
    currentSection_.setEndAddress(endAddress);
    currentSection_.setLength(ip_ - currentSection_.getStartAddress());
    addSection(currentSection_);

    return ""; // END directive doesn't produce output in first pass
}

CodeLine Assembler::getCodeLineFromSource(const std::vector<std::string>& line)
{
    return Parser::parseCodeLine(line);
}

CodeLine Assembler::getCodeLineFromFirstPass(const std::vector<std::string>& line)
{
    return Parser::parseFirstPassLine(line);
}

std::vector<std::string> Assembler::secondPass(const std::vector<std::vector<std::string>>& firstPassCode)
{
    std::vector<std::string> secondPassCode;
    secondIp_ = 0;
    int sectionIndex = 0;

    for (size_t i = 0; i < firstPassCode.size(); ++i) {
        CodeLine codeLine = getCodeLineFromFirstPass(firstPassCode[i]);
        std::string textLine;
        for (const auto& token : firstPassCode[i]) {
            textLine += token + " ";
        }
        
        std::string secondPassLine;

        // First line = start directive
        if (i == 0) {
            currentSection_ = sections_[0];
            secondIp_ = currentSection_.getStartAddress();
            
            std::stringstream ss;
            ss << "H " << codeLine.getLabel() << "\t"
               << std::hex << std::uppercase << std::setfill('0') << std::setw(6) << currentSection_.getStartAddress()
               << "\t" << std::setw(6) << currentSection_.getLength();
            secondPassLine = ss.str();
        } else {
            std::string upperCmd = codeLine.getCommand();
            std::transform(upperCmd.begin(), upperCmd.end(), upperCmd.begin(), ::toupper);
            
            // Debug: log the command being processed
            if (upperCmd.empty()) {
                throw AssemblerException("Пустая команда во втором проходе: " + textLine);
            }

            if (upperCmd == "CSECT") {
                // Handle CSECT inline (adds multiple records)
                if (currentSection_.getEndAddress() < currentSection_.getStartAddress() || 
                    currentSection_.getEndAddress() > currentSection_.getLength()) {
                    throw AssemblerException("Некорректный адрес входа в программу: " + std::to_string(currentSection_.getEndAddress()));
                }

                // Add modification records for previous section
                for (const auto& tnLine : tn_) {
                    if (tnLine.getSection() == currentSection_.getName()) {
                        std::stringstream ss;
                        ss << "M " << tnLine.getAddress() << "\t" << (tnLine.getLabel().empty() ? "" : tnLine.getLabel());
                        secondPassCode.push_back(ss.str());
                    }
                }
                
                // Add end record for previous section
                std::stringstream endSs;
                endSs << "E " << std::hex << std::uppercase << std::setfill('0') << std::setw(6) << currentSection_.getEndAddress();
                secondPassCode.push_back(endSs.str());

                // Move to next section
                sectionIndex++;
                currentSection_ = sections_[sectionIndex];
                secondIp_ = currentSection_.getStartAddress();

                // Create header record for new section
                std::stringstream ss;
                ss << "H " << codeLine.getLabel() << "\t"
                   << std::hex << std::uppercase << std::setfill('0') << std::setw(6) << currentSection_.getStartAddress()
                   << "\t" << std::setw(6) << currentSection_.getLength();
                secondPassLine = ss.str();
            } else if (upperCmd == "EXTDEF") {
                secondPassLine = processSecondPassExtdef(codeLine, textLine);
            } else if (upperCmd == "EXTREF") {
                secondPassLine = processSecondPassExtref(codeLine, textLine);
            } else if (upperCmd == "WORD") {
                secondPassLine = processSecondPassWord(codeLine);
                secondIp_ += 3;
            } else if (upperCmd == "BYTE") {
                secondPassLine = processSecondPassByte(codeLine);
            } else if (upperCmd == "RESB") {
                secondPassLine = processSecondPassResb(codeLine);
            } else if (upperCmd == "RESW") {
                secondPassLine = processSecondPassResw(codeLine);
            } else {
                // This should be a machine command (with hex opcode)
                // Check if it looks like a hex number
                bool isHexCommand = true;
                for (char c : codeLine.getCommand()) {
                    if (!std::isxdigit(c)) {
                        isHexCommand = false;
                        break;
                    }
                }
                
                if (!isHexCommand) {
                    throw AssemblerException("Неизвестная директива или команда: " + upperCmd + " в строке: " + textLine);
                }
                
                secondPassLine = processSecondPassCommand(codeLine, textLine);
            }
        }

        secondPassCode.push_back(secondPassLine);
    }

    // Add modification records and end record for the last section
    for (const auto& tnLine : tn_) {
        if (tnLine.getSection() == currentSection_.getName()) {
            std::stringstream ss;
            ss << "M " << tnLine.getAddress() << "\t" << (tnLine.getLabel().empty() ? "" : tnLine.getLabel());
            secondPassCode.push_back(ss.str());
        }
    }

    if (currentSection_.getEndAddress() < currentSection_.getStartAddress() || 
        currentSection_.getEndAddress() > currentSection_.getLength()) {
        throw AssemblerException("Некорректный адрес входа в программу: " + std::to_string(currentSection_.getEndAddress()));
    }

    std::stringstream ss;
    ss << "E " << std::hex << std::uppercase << std::setfill('0') << std::setw(6) << currentSection_.getEndAddress();
    secondPassCode.push_back(ss.str());

    return secondPassCode;
}

std::string Assembler::processSecondPassExtdef(const CodeLine& codeLine, const std::string& textLine)
{
    SymbolicName* symbolicName = getSymbolicName(codeLine.getFirstOperand(), currentSection_.getName());

    if (symbolicName == nullptr) {
        throw AssemblerException("Метка не найдена в ТСИ: " + textLine);
    }

    std::stringstream ss;
    ss << "D " << codeLine.getFirstOperand() << "\t"
       << std::hex << std::uppercase << std::setfill('0') << std::setw(6) << symbolicName->getAddress();
    return ss.str();
}

std::string Assembler::processSecondPassExtref(const CodeLine& codeLine, const std::string& textLine)
{
    SymbolicName* symbolicName = getSymbolicName(codeLine.getFirstOperand(), currentSection_.getName());

    if (symbolicName == nullptr) {
        throw AssemblerException("Метка не найдена в ТСИ: " + textLine);
    }

    std::stringstream ss;
    ss << "R " << codeLine.getFirstOperand();
    return ss.str();
}

std::string Assembler::processSecondPassWord(const CodeLine& codeLine)
{
    std::stringstream ss;
    ss << "T " << codeLine.getLabel() << " "
       << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << 3
       << " " << std::setw(6) << std::stoi(codeLine.getFirstOperand(), nullptr, 16);
    return ss.str();
}

std::string Assembler::processSecondPassByte(const CodeLine& codeLine)
{
    const std::string& operand = codeLine.getFirstOperand();

    // Check if it's a C string first
    if (isCString(operand)) {
        // Extract the string content: C"Hello!" -> Hello!
        std::string symbols = operand.substr(2, operand.length() - 3);
        int length = symbols.length();
        std::string asciiHex = convertToASCII(symbols);

        secondIp_ += length;
        
        std::stringstream ss;
        ss << "T " << codeLine.getLabel() << " "
           << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << length
           << " " << asciiHex;
        return ss.str();
    } else if (isXString(operand)) {
        std::string symbols = operand.substr(2, operand.length() - 3);
        int length = symbols.length() / 2;

        secondIp_ += length;
        
        std::stringstream ss;
        ss << "T " << codeLine.getLabel() << " "
           << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << length
           << " " << symbols;
        return ss.str();
    } else {
        // Try to parse as numeric value
        try {
            int value = std::stoi(operand, nullptr, 16);
            
            secondIp_ += 1;
            
            std::stringstream ss;
            ss << "T " << codeLine.getLabel() << " "
               << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << 1
               << " " << std::setw(2) << value;
            return ss.str();
        } catch (const std::exception&) {
            throw AssemblerException("Невозможно преобразовать первый операнд в строку или число");
        }
    }
}

std::string Assembler::processSecondPassResb(const CodeLine& codeLine)
{
    int length = std::stoi(codeLine.getFirstOperand(), nullptr, 16);

    secondIp_ += length;
    
    std::stringstream ss;
    ss << "T " << codeLine.getLabel() << " "
       << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << length;
    return ss.str();
}

std::string Assembler::processSecondPassResw(const CodeLine& codeLine)
{
    int length = std::stoi(codeLine.getFirstOperand(), nullptr, 16);

    secondIp_ += length * 3;
    
    std::stringstream ss;
    ss << "T " << codeLine.getLabel() << " "
       << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << (length * 3);
    return ss.str();
}

std::string Assembler::processSecondPassCommand(const CodeLine& codeLine, const std::string& textLine)
{
    int addressingType, commandCode;
    
    try {
        int cmdValue = std::stoi(codeLine.getCommand(), nullptr, 16);
        addressingType = cmdValue & 0x03;
        commandCode = (cmdValue & 0xFC) >> 2;
    } catch (const std::exception& e) {
        throw AssemblerException("Неизвестная команда: " + textLine);
    }

    // Find command by code
    auto cmdIt = std::find_if(availableCommands_.begin(), availableCommands_.end(),
                              [commandCode](const Command& cmd) {
                                  return cmd.getCode() == commandCode;
                              });
    
    if (cmdIt == availableCommands_.end()) {
        throw AssemblerException("Неизвестная команда: " + textLine);
    }
    
    const Command& command = *cmdIt;

    switch (addressingType) {
    case 0:
        if (!codeLine.hasFirstOperand() && !codeLine.hasSecondOperand()) {
            // Operandless command
            secondIp_ += command.getLength();
            std::stringstream ss;
            ss << "T " << codeLine.getLabel() << "\t"
               << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << command.getLength()
               << "\t" << codeLine.getCommand();
            return ss.str();
        } else if (codeLine.hasSecondOperand()) {
            // Registers
            secondIp_ += command.getLength();
            std::stringstream ss;
            ss << "T " << codeLine.getLabel() << "\t"
               << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << command.getLength()
               << "\t" << codeLine.getCommand()
               << std::hex << std::uppercase << std::setfill('0') << std::setw(1) << (getRegisterNumber(codeLine.getFirstOperand()) - 1)
               << std::setw(1) << (getRegisterNumber(codeLine.getSecondOperand()) - 1);
            return ss.str();
        } else {
            // One operand
            secondIp_ += command.getLength();
            std::stringstream ss;
            ss << "T " << codeLine.getLabel() << "\t"
               << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << command.getLength()
               << "\t" << codeLine.getCommand() << codeLine.getFirstOperand();
            return ss.str();
        }

    case 1:
    {
        SymbolicName* symbolicName = getSymbolicName(codeLine.getFirstOperand(), currentSection_.getName());
        if (symbolicName == nullptr) {
            throw AssemblerException("Метка не найдена в ТСИ: " + textLine);
        }

        secondIp_ += 4;
        
        std::stringstream ss;
        ss << "T " << codeLine.getLabel() << "\t"
           << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << command.getLength()
           << "\t" << codeLine.getCommand();
           
        if (symbolicName->getType() == "ВС") {
            ss << std::setw(6) << 0;
        } else {
            ss << std::setw(6) << symbolicName->getAddress();
        }
        
        pushToTN(codeLine.getLabel(), symbolicName->getName(), currentSection_.getName());
        
        return ss.str();
    }

    case 2:
    {
        // Relative addressing [LABEL]
        // Extract label from [LABEL]
        std::string labelName = codeLine.getFirstOperand().substr(1, codeLine.getFirstOperand().length() - 2);
        
        SymbolicName* symbolicName = getSymbolicName(labelName, currentSection_.getName());
        if (symbolicName == nullptr) {
            throw AssemblerException("Метка не найдена в ТСИ: " + textLine);
        }
        
        if (symbolicName->getType() == "ВС") {
            throw AssemblerException("Относительная адресация недопустима для внешних ссылок: " + textLine);
        }

        secondIp_ += 4;
        
        // Calculate relative offset
        int relativeOffset = symbolicName->getAddress() - secondIp_;
        
        std::stringstream ss;
        ss << "T " << codeLine.getLabel() << "\t"
           << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << command.getLength()
           << "\t" << codeLine.getCommand();
        
        // Handle negative offsets for two's complement representation
        if (relativeOffset < 0) {
            // Convert to 24-bit two's complement
            unsigned int unsignedOffset = (1 << 24) + relativeOffset;
            ss << std::setw(6) << (unsignedOffset & 0xFFFFFF);
        } else {
            ss << std::setw(6) << relativeOffset;
        }
        
        return ss.str();
    }

    default:
        throw AssemblerException("Неизвестный тип адресации");
    }
}
