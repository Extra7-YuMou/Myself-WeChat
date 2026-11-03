#include "contactuserlist.h"
#include "global.h"
#include "grouptipitem.h"
#include <QRandomGenerator>
#include <QEvent>
#include <QWheelEvent>
#include <QScrollBar>
#include "usermgr.h"
#include <QDir>

ContactUserList::ContactUserList(QWidget *parent) : QListWidget(parent)
{
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    this->viewport()->installEventFilter(this);
    connect(this, &QListWidget::itemClicked, this, &ContactUserList::slot_item_clicked);

    addContactUsers();
}

void ContactUserList::addContactUsers()
{
    auto* tip_new_friend = new GroupTipItem();
    tip_new_friend->setTip("新的朋友");
    QListWidgetItem* item = new QListWidgetItem;
    item->setSizeHint(tip_new_friend->sizeHint());
    this->addItem(item);
    this->setItemWidget(item, tip_new_friend);
    // 用于禁止 QListWidgetItem 被用户选中
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);

    _add_friend_item = new ContactUserItem();
    _add_friend_item->setObjectName("new_friend_item");
    _add_friend_item->setInfo(0, "新的朋友", ":/images/add_friend.png");
    _add_friend_item->setItemType(ListItemType::APPLY_FRIEND_ITEM);
    auto* add_item = new QListWidgetItem;
    add_item->setSizeHint(_add_friend_item->sizeHint());
    this->addItem(add_item);
    this->setItemWidget(add_item, _add_friend_item);
    //默认设置新的朋友申请条目被选中
    this->setCurrentItem(add_item);

    auto* tip_contact_user = new GroupTipItem();
    tip_contact_user->setTip("联系人");
    QListWidgetItem* item2 = new QListWidgetItem;
    item2->setSizeHint(tip_contact_user->sizeHint());
    this->addItem(item2);
    this->setItemWidget(item2, tip_contact_user);
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);

    auto _list = UserMgr::GetInstance()->GetFriendList();
    for(auto& item : _list){
        auto uid = item->_uid;
        auto nick = item->_nick;
        auto icon = GetDownLoadFilePrefix() + QDir::separator() +
                    "avatars" + QDir::separator() + item->_icon;

        auto* con_user_item = new ContactUserItem();
        con_user_item->setInfo(uid, nick, icon);
        QListWidgetItem* _item = new QListWidgetItem;
        _item->setSizeHint(con_user_item->sizeHint());
        this->addItem(_item);
        this->setItemWidget(_item, con_user_item);
    }
    // 创建QListWidgetItem，并设置自定义的widget
    // for(int i = 0; i < 13; i++){
    //     int randomValue = QRandomGenerator::global()->bounded(100); // 生成0到99之间的随机整数
    //     int str_i = randomValue%strs.size();
    //     int head_i = randomValue%heads.size();
    //     int name_i = randomValue%names.size();
    //     auto *con_user_wid = new ContactUserItem();
    //     con_user_wid->setInfo(0,names[name_i], heads[head_i]);
    //     QListWidgetItem *item = new QListWidgetItem;
    //     //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    //     item->setSizeHint(con_user_wid->sizeHint());
    //     this->addItem(item);
    //     this->setItemWidget(item, con_user_wid);
    // }
}

bool ContactUserList::eventFilter(QObject *o, QEvent *e)
{
    if(o == this->viewport() && e->type() == QEvent::Enter){
        this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    }else if(o == this->viewport() && e->type() == QEvent::Leave){
        this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }

    if(o == this->viewport() && e->type() == QEvent::Wheel){
        QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(e);
        int numDegrees = wheelEvent->angleDelta().y() / 8;
        int numSteps = numDegrees / 15;

        this->verticalScrollBar()->setValue(this->verticalScrollBar()->value() - numSteps);

        auto* scrollBar = this->verticalScrollBar();
        int maxValue = scrollBar->maximum();
        int currentValue = scrollBar->value();

        if(maxValue - currentValue <= 0){
            // 发送加载更多联系人信号
        }

        return true;
    }

    return QListWidget::eventFilter(o, e);
}

void ContactUserList::slot_item_clicked(QListWidgetItem *item)
{
    auto* w = this->itemWidget(item);
    if(!w){
        return;
    }

    ListItemBase* itemBase = qobject_cast<ListItemBase*>(w);
    if(!itemBase){
        return;
    }

    if(itemBase->getItemType() == ListItemType::INVALID_ITEM ||
        itemBase->getItemType() == ListItemType::GROUP_TIP_ITEM){
        return;
    }

    if(itemBase->getItemType() == ListItemType::APPLY_FRIEND_ITEM){
        qDebug() << "点击了新的朋友选项";
        emit sig_switch_apply_friend_page();
    }

    if(itemBase->getItemType() == ListItemType::CONTACT_USER_ITEM){
        emit sig_switch_friend_info_page();
    }
}
