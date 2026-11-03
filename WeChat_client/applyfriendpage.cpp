#include "applyfriendpage.h"
#include <QEvent>
#include <QVBoxLayout>
#include <QLabel>
#include "applyfriendlist.h"
#include "applyfrienditem.h"
#include <QRandomGenerator>
#include "tcpmgr.h"
#include "usermgr.h"

ApplyFriendPage::ApplyFriendPage(QWidget *parent)
    : QWidget{parent}
{
    QLabel* lb = new QLabel("   新的朋友");
    lb->setStyleSheet("border-bottom: 1px solid black;");
    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(lb);
    ApplyFriendList* list = new ApplyFriendList;
    layout->addWidget(list);
    this->setLayout(layout);

    connect(list, &ApplyFriendList::sig_show_search, [this](){
        emit sig_switch_default_page();
    });

    // 接收tcpmgr的信号
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_apply_friend_success, [this](std::shared_ptr<AddFriendApply> ai){
        std::shared_ptr<ApplyInfo> _ai(new ApplyInfo(ai->_from_uid,
                                                     ai->_desc,
                                                     ai->_icon,
                                                     ai->_nick,
                                                     ai->_sex,0));
        addNewApply(_ai);
    });
}

void ApplyFriendPage::addNewApply(std::shared_ptr<ApplyInfo> _apply_info)
{
    auto* item = new QListWidgetItem;
    auto* apply_item = new ApplyFriendItem;
    apply_item->setInfo(_apply_info);
    if(_apply_info->_status == 0){
        apply_item->setStatus(false);
    }else{
        apply_item->setStatus(true);
    }
    item->setSizeHint(QSize(250, apply_item->getHeight()));
    ApplyFriendList* list = qobject_cast<ApplyFriendList*>(this->layout()->itemAt(1)->widget());
    list->insertItem(0, item);
    list->setItemWidget(item, apply_item);
    connect(apply_item, &ApplyFriendItem::sig_auth_apply, [this](std::shared_ptr<ApplyInfo> _info){
        emit sig_add_chat_item(_info);
    });
}

void ApplyFriendPage::slot_notify_apply_friend(std::shared_ptr<AddFriendApply> ai)
{

}

