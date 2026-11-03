#include "applyfriendlist.h"
#include <QEvent>

ApplyFriendList::ApplyFriendList(QWidget* parent): QListWidget(parent)
{
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->installEventFilter(this);
}

bool ApplyFriendList::eventFilter(QObject *o, QEvent *e)
{
    if(o == this->viewport()){
        if(e->type() == QEvent::Enter){
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        }else if(e->type() == QEvent::Leave){
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        }

        if(e->type() == QEvent::MouseButtonPress){
            emit sig_show_search(false);
        }
    }

    return QListWidget::eventFilter(o, e);
}
