// src/ui_desktop/main.cpp
#include <QApplication>
#include <QStringList>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QStringList args = app.arguments();
    QString fileToOpen;

    if (args.size() >= 2) {
        fileToOpen = args[1];
    }

    MainWindow w;
    if (!fileToOpen.isEmpty()) {
        w.openFile(fileToOpen);
    }

    w.show();
    return app.exec();
}
