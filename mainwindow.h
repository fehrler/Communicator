#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "keithley2410.h"
#include <iostream>
#include <windows.h>
#include <utility>
#include <fstream>
#include <messlabor.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_Connect_clicked();

    void on_SearchCOM_clicked();

    void on_Close_clicked();

    void on_test_clicked();

    void on_IV_button_clicked();

    void on_Set_Compl_clicked();

    void on_Connect_TB_clicked();

    void on_Close_TB_clicked();

    void on_Measure_clicked();

    void on_checkBox_clicked();

    void on_Cal_addpoint_clicked();

private:
    Ui::MainWindow *ui;
    Keithley2410 SMU;
    Messlabor messpcb;
};

#endif // MAINWINDOW_H
