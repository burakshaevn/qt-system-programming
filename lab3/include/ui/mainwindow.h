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
        "    EXTDEF  D23\n"
        "    EXTDEF  D4\n"
        "    EXTREF  D2\n"
        "    EXTREF  D546\n"
        "D4  RESB    10\n"
        "D23 RESB    10\n"
        "    JMP     D2\n"
        "    SAVER1  D546\n"
        "    RESB    10\n"
        "A2  CSECT\n"
        "    EXTDEF  D42\n"
        "    EXTREF  D4\n"
        "D42 SAVER1  D4\n"
        "    INT     200\n"
        "    END";
    
    const QString relativeSample = 
        "PROG  START   0\n"
        "    EXTDEF  D23\n"
        "    EXTDEF  D4\n"
        "    EXTREF  D2\n"
        "    EXTREF  D546\n"
        "D4  RESB    10\n"
        "D23 RESB    10\n"
        "    JMP     [D4]\n"
        "    SAVER1  [D23]\n"
        "    RESB    10\n"
        "A2  CSECT\n"
        "    EXTDEF  D42\n"
        "    EXTREF  D4\n"
        "D42 SAVER1  [D42]\n"
        "    INT     200\n"
        "    END";
    
    const QString mixedSample = 
        "PROG  START   0\n"
        "    EXTDEF  D23\n"
        "    EXTDEF  D4\n"
        "    EXTREF  D2\n"
        "    EXTREF  D546\n"
        "D4  RESB    10\n"
        "D23 RESB    10\n"
        "    JMP     [D4]\n"
        "    SAVER1  D546\n"
        "    RESB    10\n"
        "A2  CSECT\n"
        "    EXTDEF  D42\n"
        "    EXTREF  D4\n"
        "D42 SAVER1  D4\n"
        "    INT     200\n"
        "    END";
};

#endif // MAINWINDOW_H