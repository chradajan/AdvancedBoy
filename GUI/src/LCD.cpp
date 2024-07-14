#include <GUI/include/LCD.hpp>
#include <QtGui/QOpenGLFunctions>
#include <QtOpenGLWidgets/QOpenGLWidget>

namespace gui
{
LCD::LCD(QWidget* parent) : QOpenGLWidget(parent)
{
}
}  // namespace gui
