#include <bit>
#include <filesystem>
#include <GUI/include/MainWindow.hpp>
#include <SDL2/SDL.h>
#include <QtCore/QtCore>
#include <QtWidgets/QApplication>
#include <QtWidgets/QtWidgets>

static_assert(std::endian::native == std::endian::little, "Host system must be little endian");
namespace fs = std::filesystem;

int main(int argv, char** args)
{
    QApplication app(argv, args);
    gui::MainWindow mainWindow;
    mainWindow.show();

    if (app.arguments().size() > 2)
    {
        mainWindow.SetBiosPath(app.arguments().at(1).toStdString());
        mainWindow.StartEmulation(app.arguments().at(2).toStdString());
    }

    return app.exec();
}
