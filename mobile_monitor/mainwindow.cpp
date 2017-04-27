#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPortInfo>
#include <QComboBox>
#include <QtGlobal>

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
}

MainWindow::~MainWindow()
{
    delete ui;
}
