#include "timerbtn.h"
#include <QMouseEvent>

TimerBtn::TimerBtn(QWidget *parent) : QPushButton(parent), _count(60){
    _timer = new QTimer(this);

    connect(_timer, &QTimer::timeout, [this](){
        --_count;
        if(_count < 0){
            _timer->stop();
            this->setText("获取");
            this->setEnabled(true);
            _count = 60;
            return;
        }
        this->setText(QString::number(_count));
    });
}

TimerBtn::~TimerBtn(){
    _timer->stop();
}

void TimerBtn::mouseReleaseEvent(QMouseEvent *e){
    if(e->button() == Qt::LeftButton){
        this->setEnabled(false);
        _timer->start(1000);
        this->setText(QString::number(_count));
        // 发送信号给外界才能被接收
        // 原本的按钮被提升了，需要额外明确发送信号
        emit clicked();
    }

    QPushButton::mouseReleaseEvent(e);
}
