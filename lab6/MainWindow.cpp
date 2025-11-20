#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "assembler/AssemblerException.h"
#include "assembler/TNLine.h"
#include "assembler/Section.h"
#include <QStringList>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Set word wrap for errors text box (Qt6 way)
    ui->Errors_TextBox->setLineWrapMode(QTextEdit::WidgetWidth);

    sourceCode = QString("PROG  START   0\n"
                        "    EXTDEF  D23\n"
                        "    EXTDEF  D4\n"
                        "    EXTREF  D2\n"
                        "    EXTREF  D546\n"
                        "D4  RESB    10\n"
                        "D23 RESB    10\n"
                        "    JMP     D2 \n"
                        "    SAVER1  D546\n"
                        "    RESB    10\n"
                        "A2  CSECT \n"
                        "    EXTDEF  D42\n"
                        "    EXTREF  D4\n"
                        "D42 SAVER1  D4\n"
                        "    INT     200\n"
                        "    END \n");

    ui->SourceCode_TextBox->setPlainText(sourceCode);

    // Initialize commands text box
    QStringList commandsText;
    for (const Command& cmd : assembler.AvailibleCommands) {
        commandsText.append(QString("%1 %2 %3").arg(cmd.Name, QString::number(cmd.Code, 16).toUpper(), QString::number(cmd.Length, 16).toUpper()));
    }
    ui->Commands_TextBox->setPlainText(commandsText.join("\n"));

    Reset();

    // Connect signals
    connect(ui->ProcessStep_Button, &QPushButton::clicked, this, &MainWindow::ProcessStep_Button_Click);
    connect(ui->Reset_Button, &QPushButton::clicked, this, &MainWindow::Reset_Button_Click);
    connect(ui->Pass_Button, &QPushButton::clicked, this, &MainWindow::Pass_Button_Click);
    connect(ui->SourceCode_TextBox, &QTextEdit::textChanged, this, &MainWindow::SourceCode_TextBox_TextChanged);
    connect(ui->Commands_TextBox, &QTextEdit::textChanged, this, &MainWindow::Commands_TextBox_TextChanged);
    connect(ui->comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::Mode_ComboBox_SelectionChanged);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::ProcessStep_Button_Click()
{
    try {
        assembler.ProcessStep();
        ui->BinaryCode_TextBox->setPlainText(assembler.BinaryCode.join("\n"));

        QStringList tsiText;
        for (const SymbolicName& sn : assembler.TSI) {
            QString addressStr;
            if (sn.Type == "ВС") {
                // Для EXTREF адрес не выводится вообще
                addressStr = "";
            } else if (sn.Address == -1) {
                // Для EXTDEF показываем FFFFFF при адресе -1
                if (sn.Type == "ВИ") {
                    addressStr = "FFFFFF";
                } else {
                    addressStr = "";
                }
            } else {
                addressStr = QString::number(sn.Address, 16).toUpper().rightJustified(6, '0');
            }
            if (addressStr.isEmpty()) {
                tsiText.append(QString("%1  %2 %3").arg(sn.Name, sn.Section, sn.Type));
            } else {
                tsiText.append(QString("%1 %2 %3 %4").arg(sn.Name, addressStr, sn.Section, sn.Type));
            }
        }
        ui->TSI_TextBox->setPlainText(tsiText.join("\n"));
        
        // Display TN
        QStringList tnText;
        for (const TNLine& tn : assembler.TN) {
            QString labelPart = tn.Label.isEmpty() ? "" : QString(" %1").arg(tn.Label);
            tnText.append(QString("%1%2 %3").arg(tn.Address, labelPart, tn.Section));
        }
        ui->TN_TextBox->setPlainText(tnText.join("\n"));
    } catch (const AssemblerException& ex) {
        ui->Errors_TextBox->setPlainText(QString("Ошибка: %1").arg(ex.getMessage()));
    }

    if (!ui->Errors_TextBox->toPlainText().isEmpty()) {
        ui->ProcessStep_Button->setEnabled(false);
    }
}

void MainWindow::Reset_Button_Click()
{
    Reset();
}

void MainWindow::Reset()
{
    if (ui->ProcessStep_Button == nullptr || ui->TSI_TextBox == nullptr || 
        ui->BinaryCode_TextBox == nullptr || ui->Errors_TextBox == nullptr) {
        return;
    }

    try {
        ui->ProcessStep_Button->setEnabled(true);
        ui->Pass_Button->setEnabled(true);
        ui->TSI_TextBox->clear();
        ui->TN_TextBox->clear();
        ui->BinaryCode_TextBox->clear();
        ui->Errors_TextBox->clear();

        QList<CommandDto> newCommands = Parser::TextToCommandDtos(ui->Commands_TextBox->toPlainText());
        QList<QList<QString>> sourceCode = Parser::ParseCode(ui->SourceCode_TextBox->toPlainText());

        assembler.Reset(sourceCode, newCommands);
    } catch (const AssemblerException& ex) {
        ui->Errors_TextBox->setPlainText(QString("Ошибка: %1").arg(ex.getMessage()));
    }

    if (!ui->Errors_TextBox->toPlainText().isEmpty()) {
        ui->ProcessStep_Button->setEnabled(false);
        ui->Pass_Button->setEnabled(false);
    }
}

void MainWindow::SourceCode_TextBox_TextChanged()
{
    QList<QList<QString>> newSourceCode = Parser::ParseCode(ui->SourceCode_TextBox->toPlainText());

    if (!Comparer::CompareSourceCodeVersions(assembler.SourceCode, newSourceCode)) {
        Reset();
    }
}

void MainWindow::Commands_TextBox_TextChanged()
{
    Reset();
}

void MainWindow::Pass_Button_Click()
{
    while (true) {
        try {
            bool hasFinished = assembler.ProcessStep();

            ui->BinaryCode_TextBox->setPlainText(assembler.BinaryCode.join("\n"));

            QStringList tsiText;
            for (const SymbolicName& sn : assembler.TSI) {
                QString addressStr;
                if (sn.Type == "ВС") {
                    // Для EXTREF адрес не выводится вообще
                    addressStr = "";
                } else if (sn.Address == -1) {
                    // Для EXTDEF показываем FFFFFF при адресе -1
                    if (sn.Type == "ВИ") {
                        addressStr = "FFFFFF";
                    } else {
                        addressStr = "";
                    }
                } else {
                    addressStr = QString::number(sn.Address, 16).toUpper().rightJustified(6, '0');
                }
                if (addressStr.isEmpty()) {
                    tsiText.append(QString("%1  %2 %3").arg(sn.Name, sn.Section, sn.Type));
                } else {
                    tsiText.append(QString("%1 %2 %3 %4").arg(sn.Name, addressStr, sn.Section, sn.Type));
                }
            }
            ui->TSI_TextBox->setPlainText(tsiText.join("\n"));
            
            // Display TN
            QStringList tnText;
            for (const TNLine& tn : assembler.TN) {
                QString labelPart = tn.Label.isEmpty() ? "" : QString(" %1").arg(tn.Label);
                tnText.append(QString("%1%2 %3").arg(tn.Address, labelPart, tn.Section));
            }
            ui->TN_TextBox->setPlainText(tnText.join("\n"));

            if (hasFinished) {
                break;
            }
        } catch (const AssemblerException& ex) {
            ui->Errors_TextBox->setPlainText(QString("Ошибка: %1").arg(ex.getMessage()));
            break;
        }

        if (!ui->Errors_TextBox->toPlainText().isEmpty()) {
            ui->ProcessStep_Button->setEnabled(false);
            break;
        }
    }
}

void MainWindow::Mode_ComboBox_SelectionChanged()
{
    int index = ui->comboBox->currentIndex();
    
    switch (index) {
        case 0:  // Straight
        {
            sourceCode = QString("PROG  START   0\n"
                            "    EXTDEF  D23\n"
                            "    EXTDEF  D4\n"
                            "    EXTREF  D2\n"
                            "    EXTREF  D546\n"
                            "D4  RESB    10\n"
                            "D23 RESB    10\n"
                            "    JMP     D2 \n"
                            "    SAVER1  D546\n"
                            "    RESB    10\n"
                            "A2  CSECT \n"
                            "    EXTDEF  D42\n"
                            "    EXTREF  D4\n"
                            "D42 SAVER1  D4\n"
                            "    INT     200\n"
                            "    END \n");
            assembler.AddressingMode = "Straight";
            ui->SourceCode_TextBox->setPlainText(sourceCode);
            break;
        }
        
        case 1:  // Relative
        {
            sourceCode = QString("PROG  START   0\n"
                            "    EXTDEF  D23\n"
                            "    EXTDEF  D4\n"
                            "    EXTREF  D2\n"
                            "    EXTREF  D546\n"
                            "D4  RESB    10\n"
                            "D23 RESB    10\n"
                            "    JMP     [D4] \n"
                            "    SAVER1  [D23]\n"
                            "    RESB    10\n"
                            "A2  CSECT \n"
                            "    EXTDEF  D42 \n"
                            "    EXTREF  D4\n"
                            "D42 SAVER1  [D42]\n"
                            "    INT     200\n"
                            "    END \n");
            assembler.AddressingMode = "Relative";
            ui->SourceCode_TextBox->setPlainText(sourceCode);
            break;
        }
        
        case 2:  // Mixed
        {
            sourceCode = QString("PROG  START   0\n"
                            "    EXTDEF  D23\n"
                            "    EXTDEF  D4\n"
                            "    EXTREF  D2\n"
                            "    EXTREF  D546\n"
                            "D4  RESB    10\n"
                            "D23 RESB    10\n"
                            "    JMP     [D4] \n"
                            "    SAVER1  D546\n"
                            "    RESB    10\n"
                            "A2  CSECT \n"
                            "    EXTDEF  D42\n"
                            "    EXTREF  D4\n"
                            "D42 SAVER1  D4\n"
                            "    INT     200\n"
                            "    END \n");
            assembler.AddressingMode = "Mixed";
            ui->SourceCode_TextBox->setPlainText(sourceCode);
            break;
        }
    }
    
    Reset();
}

