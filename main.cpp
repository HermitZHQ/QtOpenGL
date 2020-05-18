#include "mainwindow.h"

#include <QApplication>
#include <QtOpenGL/QGLFormat>
#include "OpenWidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    w.show();
    return a.exec();
}
