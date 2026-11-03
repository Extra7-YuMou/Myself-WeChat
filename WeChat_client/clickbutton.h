#ifndef CLICKBUTTON_H
#define CLICKBUTTON_H

#include <QPushButton>
#include "global.h"
#include <QEnterEvent>


class ClickButton : public QPushButton{
    Q_OBJECT
public:
    ClickButton(QWidget* parent = nullptr);
    void setState(const QString& normal, const QString& hover, const QString& press);
protected:
    virtual void mousePressEvent(QMouseEvent* e) override;
    virtual void mouseReleaseEvent(QMouseEvent* e) override;
    virtual void enterEvent(QEnterEvent *e) override; // 鼠标进入
    virtual void leaveEvent(QEvent *e) override;// 鼠标离开
private:
    QString _normal;
    QString _hover;
    QString _press;
};

#endif // CLICKBUTTON_H
