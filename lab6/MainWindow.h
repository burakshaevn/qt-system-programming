#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QPushButton>
#include "assembler/Assembler.h"
#include "helpers/Parser.h"
#include "helpers/Comparer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void ProcessStep_Button_Click();
    void Reset_Button_Click();
    void Pass_Button_Click();
    void SourceCode_TextBox_TextChanged();
    void Commands_TextBox_TextChanged();
    void Mode_ComboBox_SelectionChanged();

private:
    Ui::MainWindow *ui;
    Assembler assembler;
    QString sourceCode;

    void Reset();
};

#endif // MAINWINDOW_H

