#pragma once

#include <QtGui/QOpenGLFunctions>
#include <QtOpenGLWidgets/QOpenGLWidget>

namespace gui
{
class LCD : public QOpenGLWidget, public QOpenGLFunctions
{
    Q_OBJECT

public:
    /// @brief Initialize the LCD widget.
    /// @param parent Parent widget.
    LCD(QWidget* parent = nullptr);

private:
};
}  // namespace gui
