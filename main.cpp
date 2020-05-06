#include "mainwindow.h"
#include "dropgenerator.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("drop-shape-analysis");
    a.setOrganizationName("drop-shape-analysis");
    a.setOrganizationDomain("drop-shape-analysis.com");
    qRegisterMetaType<DropGenerator::TheoreticalModelParameters>();
    MainWindow w;
    w.show();
    return a.exec();
}
