#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle("NCHU Supermileage Monitor");
    w.show();
    w.showMaximized();
    return a.exec();
}
