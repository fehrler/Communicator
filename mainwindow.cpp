#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

//#####################
//### General Stuff ###
//#####################
void MainWindow::on_SearchCOM_clicked()
{
    std::vector<std::string> portlist = SMU.GetPortlist();
    ui->Ports->clear();
    for (std::vector<std::string>::iterator it = portlist.begin(); it != portlist.end(); it++)
    {
        ui->Ports->addItem(QString::fromStdString(*it));
    }
}

//###########################
//### Keithley 2410 Stuff ###
//###########################
void MainWindow::on_Connect_clicked()
{
    SMU.SetBaud(QSerialPort::Baud9600);
    SMU.SetDataBits(QSerialPort::Data8);
    SMU.SetParity(QSerialPort::NoParity);
    SMU.SetStopBit(QSerialPort::OneStop);
    if (ui->Ports->currentText() == "")
    {
        std::cout << "No port selected!" << std::endl;
        return;
    }
    else
    {
        SMU.SetCOMPort(ui->Ports->currentText().toStdString());
        if (!SMU.Init())
            std::cout << "Failed to open connection!" << std::endl;
        else
            std::cout << "Connected to Port \"" << ui->Ports->currentText().toStdString() << "\" for Keithley 2410" << std::endl;
    }
}

void MainWindow::on_Close_clicked()
{
    SMU.Close();
    std::cout << "Connection to Keithley 2410 closed." << std::endl;
}

void MainWindow::on_test_clicked()
{
    SMU.Write("*RST");
    SMU.Write(":SOURCE:FUNC VOLT");
    SMU.Write(":SOURCE:VOLT 1");
    SMU.Write(":TRIG:COUNT 5");
    SMU.Write(":FORM:ELEM CURR");
    SMU.Write(":SOURCE:CLE:AUTO ON");
    SMU.Write(":OUTPUT ON");
    SMU.Flush();

    SMU.Write(":READ?");
    SMU.Flush();
    QApplication::processEvents();
    std::string answer = "";
    answer = SMU.ReadLine(2000);
    std::cout << answer << " A" << std::endl;
}

void MainWindow::on_IV_button_clicked()
{
    on_Set_Compl_clicked();

    int tempstart = -1;
    if(messpcb.is_open())
        tempstart = messpcb.MeasureTemperature(ui->SensorChannel->value());

    double imax;
    imax = ui->IV_imax->value()*pow(10, -9+3*ui->IV_imax_unit->currentIndex());
    std::vector<std::pair<double, double> > curve = SMU.IVMeasurement(ui->IV_start->value(),ui->IV_end->value(),
                                                                      ui->IV_step->value(),imax,
                                                                      ui->IV_delay->value(), ui->IV_meanbase->value());

    std::fstream f;
    QString filename;
    int fileindex = 1;

    do{
        filename = QString("IV_Curve_") + QString::number(fileindex) + QString(".dat");
        f.open(filename.toStdString().c_str(), std::ios::in);
        if(!f.is_open())
            break;
        else
        {
            f.close();
            ++fileindex;
        }
    }while(1);

    int tempend = -1;
    if(messpcb.is_open())
        tempend = messpcb.MeasureTemperature(ui->SensorChannel->value());

    f.open(filename.toStdString().c_str(), std::ios::out);

    if(!f.is_open())
    {
        std::cout << "Output file \""<< filename.toStdString() << "\" could not be opened." << std::endl;
        return;
    }
    else
    {
        f << "# IV-Curve\n# Voltage (in V); Current (in A)" << std::endl;
        if(tempstart != -1 && tempend != -1)
            f << "# Temperature (in ADC-Counts): " << tempstart << " - " << tempend << std::endl;
    }

    for(auto it = curve.begin(); it != curve.end();++it) //std::vector<std::pair<double, double> >::iterator)
    {
        std::cout << "(" << it->first << " | " << it->second << ")" << std::endl;
        f << it->first << "\t" << it->second << std::endl;
    }

    f.close();
}

void MainWindow::on_Set_Compl_clicked()
{
    //static double lastcompliance = -1e10;
    double compliance;
    compliance = ui->Compliance->value() * pow(10, -9+3*ui->Compliance_Unit->currentIndex());

    //if(compliance != lastcompliance)
    //{
        SMU.Write(":SENSE:CURR:PROT " + QString::number(compliance).toStdString());
        SMU.Flush();
    //    lastcompliance = compliance;
    //}
}


//###############################
//### Temperature Board Stuff ###
//###############################

void MainWindow::on_Connect_TB_clicked()
{
    if(!messpcb.open(ui->Ports->currentText().toStdString()))
        std::cout << "Could not open Port \"" << ui->Ports->currentText().toStdString()
                  << "\"" << std::endl;
    else
        std::cout << "Port \"" << ui->Ports->currentText().toStdString() <<"\" opened for Temperature Board."
                  << std::endl;
}

void MainWindow::on_Close_TB_clicked()
{
    ui->checkBox->setCheckState(Qt::CheckState::Unchecked);
    messpcb.close();
    std::cout << "Connection to Temperature Board closed." << std::endl;
}

void MainWindow::on_Measure_clicked()
{
    int voltage = messpcb.MeasureTemperature(ui->SensorChannel->value());
    double temperature = messpcb.ConvertVoltageToTemperature(Messlabor::NTC, voltage);
    std::cout << "Measuring Sensor " << ui->SensorChannel->value() << std::endl
              << "  =>  " << voltage << std::endl
              << "   => " << temperature
              << " \xB0" << "C" << std::endl;

    ui->progressBar->setValue(temperature*10);
    ui->lcdtemp->display(temperature);
}

void MainWindow::on_checkBox_clicked()
{
    while(ui->checkBox->isChecked())
    {
        on_Measure_clicked();
        for(int i=0;i<25;++i)
        {
            Sleep(10);
            QApplication::processEvents();
        }

    }
}

void MainWindow::on_Cal_addpoint_clicked()
{
    std::fstream f;
    f.open(ui->Cal_filename->text().toStdString().c_str(),std::ios::in);
    if(!f.is_open())
    {
        f.open(ui->Cal_filename->text().toStdString().c_str(),std::ios::out | std::ios::app);
        f << "# Calibration of Sensors: " << ui->Cal_sensors->text().toStdString()
          << std::endl
          << "# Set Temperature; Sensors; Voltages;" << std::endl;
    }
    else
    {
        f.close();
        f.open(ui->Cal_filename->text().toStdString().c_str(),std::ios::out | std::ios::app);
    }

    //extract the sensors to read from the channel line edit:
    std::vector<int> sensors;
    std::string senstext = ui->Cal_sensors->text().toStdString();
    int newnumber = 0;
    for(unsigned int i = 0; i < senstext.length(); ++i)
    {
        char c = senstext.c_str()[i];
        if(c >= 48 && c <= 57)
            newnumber = newnumber * 10 + (c-48);
        else if(c == ',')
        {
            sensors.push_back(newnumber);
            newnumber = 0;
        }
    }
    if(senstext.length() > 0 && senstext.c_str()[senstext.length()-1] != ',')
        sensors.push_back(newnumber);

    if(sensors.size() > 0)
        std::cout << "Sensors to Measure: " << sensors.size() << std::endl;
    else
    {
        std::cout << "No sensors selected. Done before starting." << std::endl;
        f.close();
        return;
    }

    //write out temperature reference:
    f << ui->Cal_temp_is->value() << "\t\""
      << ui->Cal_sensors->text().toStdString() << "\"\t";

    //measure the sensors:
    std::cout << "Measuring Temperature " << ui->Cal_temp_is->value()
              << " \xB0" << "C:" << std::endl;
    for(auto it = sensors.begin(); it != sensors.end(); ++it)
    {
        int result = messpcb.MeasureTemperature(*it);

        std::cout << "  Sensor " << *it << ": " << result << std::endl;
        f << result << "\t";
    }
    f << std::endl;

    f.close();
}
