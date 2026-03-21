#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QStyleFactory>
#include "gui/mainwindow.h"
#include <iostream>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    app.setStyle(QStyleFactory::create("Fusion"));

    QFile styleFile(":/src/gui/style.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream stream(&styleFile);
        app.setStyleSheet(stream.readAll());
        styleFile.close();
    }
    else
    {
        std::cerr << "Warning: Could not load style.qss" << std::endl;
    }

    app.setWindowIcon(QIcon(":/icons/app_icon.ico"));

    MainWindow mainWindow;
	mainWindow.setWindowTitle("Decimal Computer Emulator");
	mainWindow.show();

    return app.exec();
}