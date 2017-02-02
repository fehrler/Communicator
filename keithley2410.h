#ifndef KEITHLEY2410_H
#define KEITHLEY2410_H
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QApplication>
#include <math.h>
#include <string.h>
#include <sstream>
#include <iostream>
#include <utility>

#include <windows.h>



class Keithley2410
{
public:
    Keithley2410();
    ~Keithley2410();

    enum Unit {Volt=0, Ampere=1, V=0, A=1, Ohm=2, O=2};//{Volt=0, Ampere=1, V=0, A=1, O=2};

    int Write(std::string text);
    std::string Read(unsigned int readlength = 0);
    std::string ReadLine(int timeout = 200);
    std::vector<std::string> GetPortlist();
    bool SetCOMPort(std::string port);
    bool SetBaud(QSerialPort::BaudRate baud);
    bool SetDataBits(QSerialPort::DataBits databits);
    bool SetParity(QSerialPort::Parity parity);
    bool SetStopBit(QSerialPort::StopBits stopbits);
    void Close();
    bool Init();
    bool Flush();
    void SetSource(Unit unit, double value);
    void SetOutput(bool enabled);
    void SetMeasure(Unit unit, int meanbase = 1);
    std::vector<std::pair<double, double> > IVMeasurement(double vstart, double vend, double vstep, double Imax, int delay = 10, int meanbase=1);
    double FindRange(double voltage, int sleep = 3000, bool turnoff = true);


private:
    QSerialPort COM;
    double CalcMean(std::string input, bool output = false);
};

#endif // KEITHLEY2410_H
