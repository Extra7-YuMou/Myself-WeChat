#include "clickoncelabel.h"

ClickOnceLabel::ClickOnceLabel(QWidget* parent): QLabel(parent)
{
    setCursor(Qt::PointingHandCursor);
}

void ClickOnceLabel::mousePressEvent(QMouseEvent *e)
{
    if(e->type() == QMouseEvent::MouseButtonPress && e->button() == Qt::LeftButton){
        emit clicked(this->text());
        return;
    }
    QLabel::mousePressEvent(e);
}
