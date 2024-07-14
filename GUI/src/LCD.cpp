#include <GUI/include/LCD.hpp>
#include <GUI/include/GBA.hpp>
#include <QtGui/QPainter>
#include <QtOpenGLWidgets/QOpenGLWidget>
#include <QtWidgets/QtWidgets>

namespace gui
{
LCD::LCD(QWidget* parent) : QOpenGLWidget(parent)
{
}

void LCD::paintGL()
{
    QPainter painter(this);
    auto image = QImage(GetFrameBuffer(), 240, 160, QImage::Format_RGB555);
    image.rgbSwap();
    painter.drawImage(this->rect(), image);
}
}  // namespace gui
