#include "keithley2410.h"

Keithley2410::Keithley2410()
{

}

Keithley2410::~Keithley2410()
{

}

int Keithley2410::Write(std::string text)
{
    if (text.substr(text.length()-2,2).compare("\r\n") != 0)
        text = text + "\r\n";
    COM.write(text.c_str());
    return text.length();
}

std::string Keithley2410::Read(unsigned int readlength)
{
    char* cdata = new char[readlength];
    COM.read(cdata, readlength);
    std::string data = std::string(cdata);
    delete cdata;
    return data;
}

std::string Keithley2410::ReadLine(int timeout)
{
    timeout /= 5;
    int timeoutcounter = 0;
    char* cdata = new char[500];
    std::string data = "";
    while(data.length() < 2 || data.substr(data.length()-2,2).compare("\r\n") != 0)
    {
        COM.readLine(cdata, 500);
        data = data + std::string(cdata);
        if(cdata[0] != 0)
            timeoutcounter = 0;
        else if(timeoutcounter >= timeout)
            break;
        else
            ++timeoutcounter;

        Sleep(5);
        QApplication::processEvents();
    }
    delete cdata;
    data = data.substr(0, data.length()-2);
    return data;
}

std::vector<std::string> Keithley2410::GetPortlist()
{
    QList<QSerialPortInfo> tempports = QSerialPortInfo::availablePorts();
    std::vector<std::string> ports;
    for (QList<QSerialPortInfo>::Iterator it = tempports.begin(); it != tempports.end(); ++it)
    {
        ports.push_back(it->portName().toStdString());
    }
//    tempports.~QList();

    return ports;
}

bool Keithley2410::SetCOMPort(std::string port)
{
    COM.setPortName(QString::fromStdString(port));
    return true;
}

bool Keithley2410::SetBaud(QSerialPort::BaudRate baud)
{
    return COM.setBaudRate(baud);
}

bool Keithley2410::SetDataBits(QSerialPort::DataBits databits)
{
    return COM.setDataBits(databits);
}

bool Keithley2410::SetParity(QSerialPort::Parity parity)
{
    return COM.setParity(parity);
}

bool Keithley2410::SetStopBit(QSerialPort::StopBits stopbits)
{
    return COM.setStopBits(stopbits);
}

void Keithley2410::Close()
{
    COM.close();
}

bool Keithley2410::Init()
{
    bool success = COM.open(QSerialPort::ReadWrite);
    if(success)
        Write("*RST");
    return success;
}

bool Keithley2410::Flush()
{
    bool result = COM.flush();

    result |= COM.waitForBytesWritten(1000);
    QApplication::processEvents();
    return result;
}

void Keithley2410::SetSource(Unit unit, double value)
{
    std::string order;

    switch (unit)
    {
    case Volt:
        order = ":SOURCE:FUNC VOLT";
        Write(order);
        order = ":SOURCE:VOLT " + QString::number(value).toStdString();
        Write(order);
    break;
    case Ampere:
        order = ":SOURCE:FUNC Ampere";
        Write(order);
        order = ":SOURCE:Ampere " + QString::number(value).toStdString();
        Write(order);
    break;
    case Ohm:
        return;
    }
    Flush();
}

void Keithley2410::SetOutput(bool enabled)
{
    if (enabled)
        Write(":OUTPUT ON");
    else
        Write(":OUTPUT OFF");
    Flush();
}

void Keithley2410::SetMeasure(Unit unit, int meanbase)
{
    std::string order;
    order = ":TRIG:COUNT " + QString::number(meanbase).toStdString();
    Write(order);
    switch (unit)
    {
    case Volt:
        order = ":SENS:FUNC 'VOLT'";
        Write(order);
        order = ":FORM:ELEM VOLT";
        Write(order);
        break;
    case Ampere:
        order = ":SENS:FUNC 'CURR'";
        Write(order);
        order = ":FORM:ELEM CURR";
        Write(order);
        break;
    case Ohm:
        order = ":SENS:FUNC 'RES'";
        Write(order);
        order = ":FORM:ELEM RES";
        Write(order);
        break;
    }
    Flush();
}

std::vector<std::pair<double, double> > Keithley2410::IVMeasurement(double vstart, double vend, double vstep, double Imax, int delay, int meanbase)
{
    if(meanbase < 1)
        meanbase = 1;

    std::vector<std::pair<double, double> > curve;
    curve.clear();

    SetMeasure(Ampere, meanbase);

    //Write(":ARM:COUNT INF");
    vstep = fabs(vstep);
    if (vstart > vend)
        vstep = -vstep;
    //SetOutput(1);
    //Write(":SOURCE:CLE:AUTO OFF");
    //Write(":SENSE:CURR:RANGE:AUTO OFF");

    FindRange(vstart, 1000*(fabs(vstart)+1),false);

    //Write(std::string(":SENSE:CURR:RANGE ") + QString::number(current*3).toStdString());

    double lastrange = 2e-6;
    double nextrange;

    for (double voltage = vstart; (voltage < vend)^(vstep < 0) || voltage == vend; voltage += vstep)
    {
        std::cout << "Measuring Voltage " << voltage << "V:" << std::endl;
        SetSource(Volt, voltage);
        Sleep(delay);
        Write(":READ?");
        Flush();
        //Sleep(10);
        std::string answer = "";
        answer = ReadLine(3000);
        QApplication::processEvents();
        double current = CalcMean(answer, true);
        std::cout << " => MEAN: " << current << std::endl;

        curve.push_back(std::make_pair(voltage, current));

        if(fabs(current) >= fabs(Imax))
            break;

        //change the range for the next measurement:
        //if(voltage == vstart)
        //    Write(":SENSE:CURR:RANGE:AUTO OFF");


        nextrange = fabs(current)* 20;
        if(nextrange > lastrange)
        {
            lastrange = nextrange;
            Write(std::string(":SENSE:CURR:RANGE ") + QString::number(nextrange).toStdString());   //(current*1.5).toStdString());
        }

        //Sleep(10);
    }

    SetOutput(0);

    return curve;

}

double Keithley2410::FindRange(double voltage, int sleep, bool turnoff)
{
    double range = 0.11;
    double current = 1e-25; //a hilariously small "current"

    SetMeasure(Ampere, 10);
    SetSource(Volt,voltage);

    SetOutput(1);
    Write(":SOURCE:CLE:AUTO OFF");
    Write(":SENSE:CURR:RANGE:AUTO ON");

    Sleep(sleep);

    Write(":READ?");
    Flush();
    std::string answer = "";
    answer = ReadLine(4000);
    QApplication::processEvents();
    current = CalcMean(answer);
    range = 1000 * fabs(current);
    Write(std::string(":SENSE:CURR:RANGE ") + QString::number(range).toStdString());
    Write(":SENSE:CURR:RANGE:AUTO OFF");

    int timeout = 0;
    while(range > fabs(current) * 100 && timeout++ < 10)
    {
        Write(":READ?");
        Flush();

        std::string answer = "";
        answer = ReadLine(3000);
        QApplication::processEvents();
        current = CalcMean(answer);

        range /= 10; //30 * current;
        Write(std::string(":SENSE:CURR:RANGE ") + QString::number(range).toStdString());
        Flush();
        Sleep(sleep);

        std::cout << "FindRange: range = " << range << " <=> current = " << current << std::endl;
    }

    if(turnoff)
        SetOutput(0);

    return range;
}

double Keithley2410::CalcMean(std::string input, bool output)
{
    if(output)
        std::cout << "CalcMean: \"" << input << "\"" << std::endl;

    if(input == "")
        return 0;

    std::stringstream stemp("");
    double mean = 0;
    double value;
    int countnumbers = 0;
    int lastindex = 0;
    int index = input.find(',',0);
    while(lastindex != std::string::npos)
    {
        if(lastindex == 0)
            --lastindex;
        stemp.str("");
        if(index != std::string::npos)
            stemp << input.substr(lastindex+1,index-lastindex);
        else
            stemp << input.substr(lastindex+1);
        stemp >> value;
        mean += value;
        ++countnumbers;
        lastindex = index;
        if(index != std::string::npos)
            index = input.find(',', index+1);
    }

    mean /= countnumbers;

    return mean;
}

