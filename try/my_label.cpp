#include "my_label.h"

MyLabel::MyLabel(QWidget *parent) : QLabel(parent)
{
    setMouseTracking(true);  // Track movement even without clicking
}

void MyLabel::mouseMoveEvent(QMouseEvent *ev)
{
    QPoint pos = ev->pos();
    if (pos.x() >= 0 && pos.y() >= 0 && pos.x() < width() && pos.y() < height()) {
        emit sendMousePosition(pos);
    }
}

void MyLabel::mousePressEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton) {
        emit Mouse_Pressed();
        QPoint pos = ev->pos();
        emit sendMousePosition(pos);
    }
}
