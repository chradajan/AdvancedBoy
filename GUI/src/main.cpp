#include <filesystem>
#include <GUI/include/MainWindow.hpp>
#include <SDL2/SDL.h>
#include <QtCore/QtCore>
#include <QtWidgets/QApplication>
#include <QtWidgets/QtWidgets>

namespace fs = std::filesystem;

int main(int argv, char** args)
{
    QApplication app(argv, args);
    gui::MainWindow mainWindow;
    mainWindow.show();
    return app.exec();
}
