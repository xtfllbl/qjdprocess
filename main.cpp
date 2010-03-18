#include <QtGui/QApplication>
#include "qjdprocessmainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qjdProcessMainWindow w;
    w.show();
    return a.exec();
}
