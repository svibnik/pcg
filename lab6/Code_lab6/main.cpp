#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("Letter3D Viewer - H");
    a.setOrganizationName("Lab6");

    MainWindow w;
    w.setWindowTitle("Лабораторная работа 6 - Буква H (3D)");
    w.resize(1400, 750);
    w.show();

    return a.exec();
}
