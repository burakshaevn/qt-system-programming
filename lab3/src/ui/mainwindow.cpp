#include "ui/mainwindow.h"
#include "ui_mainwindow.h"
#include <QApplication>
#include <QTextStream>
#include <QStringList>
#include <QRegularExpression>
#include <QScrollBar>
#include <QComboBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupConnections();
    initializeDefaultContent();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupConnections()
{
    connect(ui->firstPassButton, &QPushButton::clicked, this, &MainWindow::onFirstPassClicked);
    connect(ui->secondPassButton, &QPushButton::clicked, this, &MainWindow::onSecondPassClicked);
    connect(ui->exampleComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onExampleChanged);
}

void MainWindow::initializeDefaultContent()
{
    ui->sourceCodeTextEdit->setPlainText(straightSample);
    
    // Initialize commands text edit with default commands
    QString commandsText;
    for (const auto& cmd : assembler.getAvailableCommands()) {
        commandsText += QString::fromStdString(cmd.getName()) + " " 
                       + QString::number(cmd.getCode(), 16).toUpper() + " " 
                       + QString::number(cmd.getLength(), 16).toUpper() + "\n";
    }
    ui->commandsTextEdit->setPlainText(commandsText);
}

void MainWindow::onFirstPassClicked()
{
    ui->secondPassButton->setEnabled(true);
    
    try {
        // Clear previous results
        ui->tsiTextEdit->clear();
        ui->secondSetupTable->clear();
        ui->firstPassTextEdit->clear();
        ui->secondPassTextEdit->clear();
        ui->firstPassErrorsTextEdit->clear();
        ui->secondPassErrorsTextEdit->clear();
        
        // Parse commands
        QString commandsText = ui->commandsTextEdit->toPlainText();
        std::vector<std::vector<std::string>> commandLines = Parser::parseCode(commandsText.toStdString());
        std::vector<Command> commands = Parser::textToCommands(commandsText.toStdString());
        assembler.setAvailableCommands(commands);
        
        // Clear TSI, TN, and Sections
        assembler.clearTSI();
        assembler.clearTN();
        assembler.clearSections();
        
        // Parse source code
        QString sourceText = ui->sourceCodeTextEdit->toPlainText();
        std::vector<std::vector<std::string>> sourceLines = Parser::parseCode(sourceText.toStdString());
        
        // Get addressing mode from combo box
        std::string addressingMode = "Straight";
        int selectedIndex = ui->exampleComboBox->currentIndex();
        if (selectedIndex == 0) {
            addressingMode = "Straight";
        } else if (selectedIndex == 1) {
            addressingMode = "Relative";
        } else if (selectedIndex == 2) {
            addressingMode = "Mixed";
        }
        
        // First pass
        std::vector<std::string> firstPassResult = assembler.firstPass(sourceLines, addressingMode);
        
        // Display results
        QString firstPassText;
        for (const auto& line : firstPassResult) {
            firstPassText += QString::fromStdString(line) + "\n";
        }
        ui->firstPassTextEdit->setPlainText(firstPassText);
        
        // Display TSI with section and type
        QString tsiText;
        for (const auto& sym : assembler.getTSI()) {
            QString address = (sym.getAddress() >= 0) ? 
                            QString::number(sym.getAddress(), 16).toUpper().rightJustified(6, '0') : 
                            QString("");
            tsiText += QString::fromStdString(sym.getName()) + "\t" + 
                      address + "\t" +
                      QString::fromStdString(sym.getSection()) + "\t" +
                      QString::fromStdString(sym.getType()) + "\n";
        }
        ui->tsiTextEdit->setPlainText(tsiText);
        
    } catch (const AssemblerException& e) {
        ui->firstPassErrorsTextEdit->setPlainText("Ошибка: " + QString::fromStdString(e.what()));
        ui->secondPassButton->setEnabled(false);
    } catch (const std::exception& e) {
        ui->firstPassErrorsTextEdit->setPlainText("Ошибка: " + QString::fromStdString(e.what()));
        ui->secondPassButton->setEnabled(false);
    }
}

void MainWindow::onSecondPassClicked()
{
    ui->secondPassTextEdit->clear();
    ui->secondSetupTable->clear();
    ui->secondPassErrorsTextEdit->clear();
    
    if (ui->firstPassTextEdit->toPlainText().isEmpty()) {
        return;
    }
    
    try {
        // Clear TN before second pass
        assembler.clearTN();
        
        // Parse first pass result
        QString firstPassText = ui->firstPassTextEdit->toPlainText();
        std::vector<std::vector<std::string>> firstPassLines = Parser::parseCode(firstPassText.toStdString());
        
        // Second pass
        std::vector<std::string> secondPassResult = assembler.secondPass(firstPassLines);
        
        // Display results
        QString secondPassText;
        for (const auto& line : secondPassResult) {
            secondPassText += QString::fromStdString(line) + "\n";
        }
        ui->secondPassTextEdit->setPlainText(secondPassText);
        
        // Display TN with label and section
        QString tnText;
        for (const auto& tnLine : assembler.getTN()) {
            tnText += QString::fromStdString(tnLine.getAddress()) + "\t" +
                     (tnLine.getLabel().empty() ? QString("") : QString::fromStdString(tnLine.getLabel())) + "\t" +
                     QString::fromStdString(tnLine.getSection()) + "\n";
        }
        ui->secondSetupTable->setPlainText(tnText);
        
    } catch (const AssemblerException& e) {
        ui->secondPassErrorsTextEdit->setPlainText("Ошибка: " + QString::fromStdString(e.what()));
    } catch (const std::exception& e) {
        ui->secondPassErrorsTextEdit->setPlainText("Ошибка: " + QString::fromStdString(e.what()));
    }
}

void MainWindow::onExampleChanged(int index)
{
    switch (index) {
    case 0:
        ui->sourceCodeTextEdit->setPlainText(straightSample);
        break;
    case 1:
        ui->sourceCodeTextEdit->setPlainText(relativeSample);
        break;
    case 2:
        ui->sourceCodeTextEdit->setPlainText(mixedSample);
        break;
    default:
        break;
    }
}