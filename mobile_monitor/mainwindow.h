#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPortInfo>
#include <QComboBox>
#include <QtGlobal>
#include <QSerialPort>

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
    void on_connect_button_clicked();
    void serialRead();

private:
    Ui::MainWindow *ui;
    QSerialPort serial;
    bool serial_connected;
};

#endif // MAINWINDOW_H
