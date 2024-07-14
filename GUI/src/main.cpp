#include <filesystem>
#include <GUI/include/MainWindow.hpp>
#include <QtCore/QtCore>
#include <QtWidgets/QApplication>

namespace fs = std::filesystem;

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    gui::MainWindow mainWindow;
    mainWindow.show();
    return app.exec();
}
