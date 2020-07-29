#include "mainwindow.h"

#include <QApplication>
#include <QSharedMemory>

int main(int argc, char *argv[])
{
    int nExitCode = 0;
    QSharedMemory shared("onlyone");
    if(shared.attach()){
        return 0;
    }
    shared.create(1);

    do
    {
        QApplication a(argc, argv);
        MainWindow w;
        w.resize(1024 , 768);
        w.show();
        nExitCode = a.exec();
    } while(nExitCode == MainWindow::EXIT_CODE_REBOOT);

    return nExitCode;
}
