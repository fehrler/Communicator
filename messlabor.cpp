#include "messlabor.h"

Messlabor::Messlabor()
{
    init();
}

Messlabor::Messlabor(std::string port)
{
    connection.setPortName(QString(port.c_str()));
    init();
}

std::vector<std::string> Messlabor::ScanPorts()
{
    QList<QSerialPortInfo> availableports = QSerialPortInfo::availablePorts();
    std::vector<std::string> portlist;

    for(auto it = availableports.begin(); it != availableports.end(); ++it)
        portlist.push_back(it->portName().toStdString());

    return portlist;
}

void Messlabor::SetCOMPort(std::string port)
{
    connection.setPortName(QString(port.c_str()));
}

bool Messlabor::open(std::string port)
{
    if(port == "")
        return connection.open(QSerialPort::ReadWrite);
    else
    {
        connection.setPortName(QString(port.c_str()));
        return connection.open(QSerialPort::ReadWrite);
    }
}

bool Messlabor::open(QSerialPortInfo port)
{
    connection.setPort(port);
    return connection.open(QSerialPort::ReadWrite);
}

void Messlabor::close()
{
    connection.close();
}

int Messlabor::MeasureTemperature(char channel)
{
    //on one PCB only 16 temperature sensors are available
    if(channel >= 16)
        return -1;

    char command[2] = {1,channel};
    connection.write(command, 2);
    connection.flush();
    QApplication::processEvents();

    Sleep(100);

    int timeout = 0;
    char answer[5] = {0};
    int readbytes = 0;
    while(readbytes == 0 && timeout++ < 800)
    {
        readbytes = connection.read(answer, 5);

        Sleep(5);
        QApplication::processEvents();
    }


    if(readbytes < 2)
        return -1;
    else
    {
        std::cout << " => " << short((unsigned char) answer[0]) << " "
                  << short((unsigned char) answer[1]) << std::endl;
        return ((unsigned char)answer[0]) * 256 + ((unsigned char)answer[1]);
    }
}

double Messlabor::ConvertVoltageToTemperature(Messlabor::Sensortype type, int voltage)
{
    double temperature = -1e10;
    switch(type)
    {
    case(NTC):
        //conversion for voltage \in [0,5115] (5 times the ADC range)
        {
        temperature = voltage / 1023.;  // / 5times * 5V
        const double parameter[4] = {-1.86128,
                                      15.6115,
                                     -66.7321,
                                      145.505};
        temperature = parameter[0] * pow(temperature, 3)
                    + parameter[1] * pow(temperature, 2)
                    + parameter[2] * temperature
                    + parameter[3];
        }
        break;
    case(PT100):
        //TODO
        break;
    case(PT1000):
        //TODO
        break;
    default:
        break;
    }


    return temperature;
}

void Messlabor::init()
{
    connection.setBaudRate(QSerialPort::Baud9600);
    connection.setParity(QSerialPort::NoParity);
    connection.setDataBits(QSerialPort::Data8);
    connection.setStopBits(QSerialPort::OneStop);
}
