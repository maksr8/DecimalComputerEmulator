#include <QApplication>
#include <QWidget>
#include "core/Computer.h"
#include "gui/mainwindow.h"
#include <iostream>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    Computer computer{};

    std::vector<int> program{18000, 11900, 18000, 20900, 19000, 0};
	// INP, STA 900, INP, ADD 900, OUT, HLT

    computer.loadProgram(program);

    while (!computer.getCPU().isHalted())
    {
        if (computer.getCPU().isWaitingForInput())
        {
            int inputVal;
            std::cout << "Input: ";
            std::cin >> inputVal;
            computer.provideInput(inputVal);
        }
        else
        {
            computer.step();
        }
    }

    std::cout << "Program halted" << std::endl;

    for (int val : computer.getCPU().getOutputBuffer())
    {
        std::cout << "Ouput buffer: " << val << std::endl;
    }

 //   MainWindow mainWindow{};
	//mainWindow.show();

    return app.exec();
}