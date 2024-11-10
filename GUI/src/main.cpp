#include <bit>
#include <filesystem>
#include <GUI/include/MainWindow.hpp>
#include <QtWidgets/QApplication>

static_assert(std::endian::native == std::endian::little, "Host system must be little endian");
namespace fs = std::filesystem;

int main(int argv, char** args)
{
    QApplication app(argv, args);
    QCoreApplication::setApplicationName("AdvancedBoy");
    gui::MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
