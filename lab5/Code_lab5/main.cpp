#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    w.setWindowTitle("Алгоритмы отсечения отрезков и многоугольников");
    w.resize(1200, 700);
    w.show();

    return a.exec();
}
