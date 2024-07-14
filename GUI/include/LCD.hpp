#pragma once

#include <QtOpenGLWidgets/QOpenGLWidget>

namespace gui
{
/// @brief Class representing the GBA LCD screen.
class LCD : public QOpenGLWidget
{
    Q_OBJECT

public:
    /// @brief Initialize the LCD widget.
    /// @param parent Parent widget.
    LCD(QWidget* parent = nullptr);

private:
    /// @brief Update the screen with the most recently completed frame.
    void paintGL() override;
};
}  // namespace gui
