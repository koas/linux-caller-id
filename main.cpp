#include <QtGui/QApplication>
#include "configWindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    configWindow w;
    w.show();

    return a.exec();
}
