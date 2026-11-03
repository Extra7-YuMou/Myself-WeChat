#include "chatuserlist.h"
#include <QEvent>
#include <QWheelEvent>
#include <QScrollBar>
#include "ChatUserItem.h"
#include "tcpmgr.h"
#include <QListWidgetItem>

ChatUserList::ChatUserList(QWidget *parent) : QListWidget(parent){
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->viewport()->installEventFilter(this);
}

bool ChatUserList::eventFilter(QObject *watched, QEvent *e)
{
    if(watched == this->viewport()){
        if(e->type() == QEvent::Enter) {
            setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        } else if(e->type() == QEvent::Leave) {
            setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        }
    }

    if(watched == this->viewport() && e->type() == QEvent::Wheel){
        QWheelEvent* event = static_cast<QWheelEvent*>(e);

        int numDegrees = event->angleDelta().y() / 8;
        int numSteps = numDegrees / 15; // 计算滚动步数

        // 设置滚动幅度
        this->verticalScrollBar()->setValue(this->verticalScrollBar()->value() - numSteps);

        // 检查是否滚动到底部
        QScrollBar* scrollBar = this->verticalScrollBar();
        int maxScrollValue = scrollBar->maximum();
        int currentValue = scrollBar->value();

        // if(maxScrollValue - currentValue <= 0){
        //     qDebug() << "load more chat user";
        // }

        return true;
    }

    return QListWidget::eventFilter(watched, e);
}
