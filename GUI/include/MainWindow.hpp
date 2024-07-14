#pragma once

#include <GUI/include/LCD.hpp>
#include <QtWidgets/QMainWindow>

namespace gui
{
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /// @brief Initialize GUI.
    /// @param parent Parent widget.
    MainWindow(QWidget* parent = nullptr);

private:
    LCD screen_;
};
}  // namespace gui
