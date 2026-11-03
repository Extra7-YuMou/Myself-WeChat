#include "clickbutton.h"

ClickButton::ClickButton(QWidget *parent) : QPushButton(parent){}

void ClickButton::setState(const QString &normal, const QString &hover, const QString &press){
    _normal = normal;
    _hover = hover;
    _press = press;
    setProperty("state", "normal");
    repolish(this);
    update();
}

void ClickButton::mousePressEvent(QMouseEvent *e)
{
    setProperty("state", _press);
    repolish(this);
    update();
    QPushButton::mousePressEvent(e);
}

void ClickButton::mouseReleaseEvent(QMouseEvent *e)
{
    setProperty("state", _hover);
    repolish(this);
    update();
    QPushButton::mouseReleaseEvent(e);
}

void ClickButton::enterEvent(QEnterEvent *e)
{
    setProperty("state", _hover);
    repolish(this);
    update();
    QPushButton::enterEvent(e);
}

void ClickButton::leaveEvent(QEvent *e)
{
    setProperty("state", _normal);
    repolish(this);
    update();
    QPushButton::leaveEvent(e);
}

