#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QWidget>
#include <QMessageBox>
#include "assembler/assembler.h"
#include "parser/parser.h"

QT_BEGIN_NAMESPACE
class QTextEdit;
class QPushButton;
QT_END_NAMESPACE

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onFirstPassClicked();
    void onSecondPassClicked();
    void onExampleChanged(int index);

private:
    void setupConnections();
    void initializeDefaultContent();
    
    Ui::MainWindow *ui;
    
    // Assembler instance
    Assembler assembler;
    
    // Sample source codes for different addressing modes
    const QString straightSample = 
        "PROG  START   0\n"
        "    JMP     L1\n"
        "A1  RESB    10\n"
        "A2  RESW    20\n"
        "B1  WORD    4096\n"
        "B2  BYTE    X\"2F4C008A\"\n"
        "B3  BYTE    C\"Hello!\"\n"
        "B4  BYTE    128\n"
        "L1  LOADR1  B1\n"
        "    LOADR2  B4\n"
        "    ADD     R1  R2\n"
        "    SAVER1  L1\n"
        "    INT     200\n"
        "    END";
    
    const QString relativeSample = 
        "PROG  START   0\n"
        "    JMP     [L1]\n"
        "A1  RESB    10\n"
        "A2  RESW    20\n"
        "B1  WORD    4096\n"
        "B2  BYTE    X\"2F4C008A\"\n"
        "B3  BYTE    C\"Hello!\"\n"
        "B4  BYTE    128\n"
        "L1  LOADR1  [B1]\n"
        "    LOADR2  [B4]\n"
        "    ADD     R1  R2\n"
        "    SAVER1  [L1]\n"
        "    INT     200\n"
        "    END";
    
    const QString mixedSample = 
        "PROG  START   0\n"
        "    JMP     [L1]\n"
        "A1  RESB    10\n"
        "A2  RESW    20\n"
        "B1  WORD    4096\n"
        "B2  BYTE    X\"2F4C008A\"\n"
        "B3  BYTE    C\"Hello!\"\n"
        "B4  BYTE    128\n"
        "L1  LOADR1  B1\n"
        "    LOADR2  [B4]\n"
        "    ADD     R1  R2\n"
        "    SAVER1  L1\n"
        "    INT     200\n"
        "    END";
};

#endif // MAINWINDOW_H