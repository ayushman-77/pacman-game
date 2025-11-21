#ifndef MY_LABEL_H
#define MY_LABEL_H

#include <QLabel>
#include <QMouseEvent>
#include <QPoint>

class MyLabel : public QLabel
{
    Q_OBJECT

public:
    explicit MyLabel(QWidget *parent = nullptr);

protected:
    void mouseMoveEvent(QMouseEvent *ev) override;
    void mousePressEvent(QMouseEvent *ev) override;

signals:
    void sendMousePosition(QPoint &pos);  // Emits current mouse position
    void Mouse_Pressed();                 // Emits when mouse is clicked
};

#endif // MY_LABEL_H
