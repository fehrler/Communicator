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

    f.open(filename.toStdString().c_str(), std::ios::out);

    if(!f.is_open())
    {
        std::cout << "Output file \""<< filename.toStdString() << "\" could not be opened." << std::endl;
        return;
    }
    else
        f << "# IV-Curve\n# Voltage (in V); Current (in A)" << std::endl;

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
    ui->temperature->setText(QString::number(temperature,'g',3));
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
