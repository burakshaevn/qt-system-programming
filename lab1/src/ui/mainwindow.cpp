#include "ui/mainwindow.h"
#include "ui_mainwindow.h"
#include <QApplication>
#include <QTextStream>
#include <QStringList>
#include <QRegularExpression>
#include <QScrollBar>

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
}

void MainWindow::initializeDefaultContent()
{
    ui->sourceCodeTextEdit->setPlainText(defaultSourceCode);
    
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
        ui->firstPassTextEdit->clear();
        ui->firstPassErrorsTextEdit->clear();
        
        // Parse commands
        QString commandsText = ui->commandsTextEdit->toPlainText();
        std::vector<std::vector<std::string>> commandLines = Parser::parseCode(commandsText.toStdString());
        std::vector<Command> commands = Parser::textToCommands(commandsText.toStdString());
        assembler.setAvailableCommands(commands);
        
        // Clear TSI
        assembler.clearTSI();
        
        // Parse source code
        QString sourceText = ui->sourceCodeTextEdit->toPlainText();
        std::vector<std::vector<std::string>> sourceLines = Parser::parseCode(sourceText.toStdString());
        
        // First pass
        std::vector<std::string> firstPassResult = assembler.firstPass(sourceLines);
        
        // Display results
        QString firstPassText;
        for (const auto& line : firstPassResult) {
            firstPassText += QString::fromStdString(line) + "\n";
        }
        ui->firstPassTextEdit->setPlainText(firstPassText);
        
        // Display TSI
        QString tsiText;
        for (const auto& sym : assembler.getTSI()) {
            tsiText += QString::fromStdString(sym.getName()) + " " + 
                      QString::number(sym.getAddress(), 16).toUpper().rightJustified(6, '0') + "\n";
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
    ui->secondPassErrorsTextEdit->clear();
    
    if (ui->firstPassTextEdit->toPlainText().isEmpty()) {
        return;
    }
    
    try {
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
        
    } catch (const AssemblerException& e) {
        ui->secondPassErrorsTextEdit->setPlainText("Ошибка: " + QString::fromStdString(e.what()));
    } catch (const std::exception& e) {
        ui->secondPassErrorsTextEdit->setPlainText("Ошибка: " + QString::fromStdString(e.what()));
    }
}