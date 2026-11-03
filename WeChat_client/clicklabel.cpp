#include "clicklabel.h"
#include <QMouseEvent>

ClickLabel::ClickLabel(QWidget *parent): QLabel(parent), state(ClickState::normal){}

void ClickLabel::mousePressEvent(QMouseEvent *event){
    if(event->button() == Qt::LeftButton){
        if(state == ClickState::normal){
            state = ClickState::selected;
            setProperty("state", _select_hover);
            repolish(this);
            // 设置后需要使用update函数更新样式，否则不生效
            update();
        }else{
            state = ClickState::normal;
            setProperty("state", _normal_hover);
            repolish(this);
            update();
        }
        emit clicked(this->text(), state);
    }

    QLabel::mousePressEvent(event);
}

void ClickLabel::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == QMouseEvent::MouseButtonRelease){
        if(state == ClickState::normal){
            setProperty("state", _normal_hover);
            repolish(this);
            update();
        }else{
            setProperty("state", _select_hover);
            repolish(this);
            update();
        }
    }
    QLabel::mouseReleaseEvent(event);
}

void ClickLabel::enterEvent(QEnterEvent *event){
    if(event->type() == QMouseEvent::Enter){
        if(state == ClickState::normal){
            setProperty("state", _normal_hover);
            repolish(this);
            update();
        }else{
            setProperty("state", _select_hover);
            repolish(this);
            update();
        }
    }

    QLabel::enterEvent(event);
}

void ClickLabel::leaveEvent(QEvent *event){
    if(event->type() == QMouseEvent::Leave){
        if(state == ClickState::normal){
            setProperty("state", _normal);
            repolish(this);
            update();
        }else{
            setProperty("state", _select);
            repolish(this);
            update();
        }
    }
    QLabel::leaveEvent(event);
}

void ClickLabel::setState(QString normal, QString normal_hover, QString normal_press, QString select, QString select_hover, QString select_press)
{
    _normal = normal;
    _normal_hover = normal_hover;
    _normal_press = normal_press;

    _select = select;
    _select_hover = select_hover;
    _select_press = select_press;

    setProperty("state", normal);
    repolish(this);
}

ClickState ClickLabel::GetState(){
    return state;
}

void ClickLabel::setType(SideType type)
{
    _type = type;
}

SideType ClickLabel::getType()
{
    return _type;
}

void ClickLabel::setCurState(ClickState _state)
{
    state = _state;
    if(state == ClickState::normal){
        setProperty("state", _normal);
        repolish(this);
        update();
    }else{
        setProperty("state", _select);
        repolish(this);
        update();
    }
}
