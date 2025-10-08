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

private:
    void setupConnections();
    void initializeDefaultContent();
    
    Ui::MainWindow *ui;
    
    // Assembler instance
    Assembler assembler;
    
    // Default source code
    const QString defaultSourceCode = 
        "PROG  START   100\n"
        "    JMP     L1 \n"
        "B1  WORD    40 \n"
        "B3  BYTE    C\"Hello!\"\n"
        "B4  BYTE    12\t\n"
        "L1  LOADR1  B1\t\n"
        "    LOADR2  B4\n"
        "    ADD R1  R2\n"
        "    SAVER1  B1\n"
        "    INT     200\t\n"
        "    END";
};

#endif // MAINWINDOW_H