#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPortInfo>
#include <QComboBox>
#include <QtGlobal>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    /* list all available COM ports */
    Q_FOREACH(QSerialPortInfo port, QSerialPortInfo::availablePorts()) {
        ui->serial_combo->addItem(port.portName());
    }

    /* Add baudrate options */
    ui->baudrate_combo->addItem("115200");
    ui->baudrate_combo->addItem("57600");

    connect(&serial,SIGNAL(readyRead()),this,SLOT(serialRead()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_connect_button_clicked()
{
    if(serial_connected == false) {
        if(ui->baudrate_combo->currentText().toStdString() == "57600") {
            serial.setBaudRate(QSerialPort::Baud57600);
        } else if(ui->baudrate_combo->currentText().toStdString() == "115200") {
            serial.setBaudRate(QSerialPort::Baud115200);
        } else {
            QMessageBox::information(NULL, "error", "unsupported baudrate", QMessageBox::Yes);
            return;
        }

        serial.setPortName(ui->serial_combo->currentText());
        serial.setDataBits(QSerialPort::Data8);              //Data bits
        serial.setParity(QSerialPort::NoParity);             //No parity
        serial.setStopBits(QSerialPort::OneStop);            //No stop bit
        serial.setFlowControl(QSerialPort::NoFlowControl);   //No control

        if(serial.open(QIODevice::ReadWrite)) {
            serial_connected = true;
            ui->connect_button->setText("Disconnect");
        } else {
            QMessageBox::information(NULL, "error", "Failed to open Serial Port", QMessageBox::Yes);
        }

        qDebug("Open COM");
    } else {
        serial.close();
        serial_connected = false;
        ui->connect_button->setText("Connect");
    }
}

void MainWindow::serialRead()
{
    QByteArray data = serial.read(1);

    if(data.at(0) == '@') {
        //Engine RPM
        data = serial.read(4);
        ui->rpm_label->setText(data);
        //qDebug(data);

        //Car Speed
        data = serial.read(3);
        ui->speed_label->setText(data + "km/hr");
        //qDebug(data);

        //A/F
        data = serial.read(4);
        ui->af_label->setText(data);
        //qDebug(data);

        //Engine Temp
        data = serial.read(3);
        ui->engine_tmp_label->setText(data + "°C");
        //qDebug(data);

        QPalette  on, off;
        on.setColor(QPalette::WindowText, Qt::green);
        off.setColor(QPalette::WindowText, Qt::red);

        /* Turn off indicator */
        data = serial.read(1);
        if(data.toInt() == 0) {
            ui->turn_off_indicator->setPalette(off);
        } else {
            ui->turn_off_indicator->setPalette(on);
        }

        /* Turn on indicator */
        data = serial.read(1);
        if(data.toInt() == 0) {
            ui->turn_on_indicator->setPalette(off);
        } else {
            ui->turn_on_indicator->setPalette(on);
        }
    }
}

void MainWindow::on_connect_button_2_clicked()
{
    for(int i = 0; i < ui->serial_combo->count(); i++) {
        ui->serial_combo->removeItem(i);
    }

    /* list all available COM ports */
    Q_FOREACH(QSerialPortInfo port, QSerialPortInfo::availablePorts()) {
        ui->serial_combo->addItem(port.portName());
    }
}
