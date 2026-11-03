#ifndef TIMERBTN_H
#define TIMERBTN_H

#include <QPushButton>
#include <QTimer>

class TimerBtn : public QPushButton
{
    Q_OBJECT
public:
    TimerBtn(QWidget* parent = nullptr);
    ~TimerBtn();
    virtual void mouseReleaseEvent(QMouseEvent* e);
private:
    QTimer* _timer;
    int _count;
};

#endif // TIMERBTN_H
