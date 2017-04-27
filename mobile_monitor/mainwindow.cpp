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
    ui->baudrate_combo->addItem("57600");
    ui->baudrate_combo->addItem("115200");

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
    QByteArray data = serial.readAll();

    qDebug(data);
}
