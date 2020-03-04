#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setOrganizationName("Cyril Margorin");
    a.setOrganizationDomain("tower.pp.ru");
    a.setApplicationName("RandomPlayer");

    MainWindow w;
    w.show();
    return a.exec();
}
