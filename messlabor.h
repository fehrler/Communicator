#ifndef MESSLABOR_H
#define MESSLABOR_H

#include <string>
#include <vector>
#include <iostream>
#include <bitset>

#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QList>
#include <QApplication>
#include <windows.h>

/**
 * @brief The Messlabor class is supposed to work with the
 *          Messlabor v1(8) PCB with the Software from the
 *          28.01.2017 neglecting the RS485 capability of
 *          the board.
 */
class Messlabor
{
public:
    enum Sensortype {NTC=0, PT100=1, PT1000=2};

    Messlabor();
    Messlabor(std::string port);

    std::vector<std::string> ScanPorts();

    void SetCOMPort(std::string port);
    bool open(std::string port = "");
    bool open(QSerialPortInfo port);
    void close();

    int MeasureTemperature(char channel);
    double ConvertVoltageToTemperature(Sensortype type, int voltage);

private:
    void init();

    QSerialPort connection;


};

#endif // MESSLABOR_H
