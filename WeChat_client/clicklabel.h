#ifndef CLICKLABEL_H
#define CLICKLABEL_H

#include <QLabel>
#include "global.h"

class ClickLabel : public QLabel
{
    Q_OBJECT
public:
    ClickLabel(QWidget* parent = nullptr);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void enterEvent(QEnterEvent *event);
    virtual void leaveEvent(QEvent *event);
    void setState(QString normal = "", QString normal_hover = "", QString normal_press = "",
                  QString select = "", QString select_hover = "", QString select_press = "");
    ClickState GetState();
    void setStatusState(ClickState _state) { state = _state;}
    void setType(SideType type);
    SideType getType();
    void setCurState(ClickState _state);
signals:
    void clicked(QString, ClickState);

private:
    SideType _type;

    QString _normal;
    QString _normal_hover;
    QString _normal_press;

    QString _select;
    QString _select_hover;
    QString _select_press;

    ClickState state;
};

#endif // CLICKLABEL_H
