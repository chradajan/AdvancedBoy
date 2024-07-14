#include <GUI/include/MainWindow.hpp>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QtWidgets>

namespace gui
{
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), screen_(this)
{
    // Initialize screen
    setCentralWidget(&screen_);
}
}  // namespace gui
