#include "searchlist.h"
#include <QEvent>
#include <QWheelEvent>
#include <QScrollBar>
#include "AddUserItem.h"
#include <QListWidgetItem>
#include <QDebug>
#include <QObject>
#include "CustomizeEdit.h"
#include "tcpmgr.h"
#include "userdata.h"
#include <memory>
#include <QMessageBox>
#include "usermgr.h"

SearchList::SearchList(QWidget* parent): QListWidget(parent), _find_dlg(nullptr)
{
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    addTipItem();

    connect(this, &QListWidget::itemClicked, this, &SearchList::slot_item_clicked);
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_search_user_success, this, &SearchList::slot_search_user_success);

    initHandlers();
}

void SearchList::setSearchEdit(QWidget *search_edit)
{
    if(search_edit){
        _search_edit = search_edit;
    }
}

void SearchList::initHandlers()
{
    _handlers[ReqId::ID_SEARCH_USER_RSP] = [this](std::shared_ptr<SearchInfo> si){
        _find_dlg = std::make_shared<FindResultDialog>();
        (std::dynamic_pointer_cast<FindResultDialog>(_find_dlg))->setSearchInfo(si);
        _find_dlg->setSearchInfo(si);
        auto friend_list = UserMgr::GetInstance()->GetFriendList();
        if(friend_list.find(si->_uid) != friend_list.end()){
            _find_dlg->setAddBtnText();
        }
        _find_dlg->show();
    };
}

bool SearchList::eventFilter(QObject *o, QEvent *e)
{
    if(o == this->viewport() && e->type() == QEvent::Enter){
        this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    }else if(o == this->viewport() && e->type() == QEvent::Leave){
        this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }

    if(o == this->viewport() && e->type() == QEvent::Wheel){
        QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(e);
        int numDegress = wheelEvent->angleDelta().y() / 8;
        int numSteps = numDegress / 15;

        this->verticalScrollBar()->setValue(this->verticalScrollBar()->value() - numSteps);
        return true;
    }

    return QListWidget::eventFilter(o, e);
}

void SearchList::addTipItem()
{
    // 用来做上边界分割的
    auto *invalid_item = new QWidget();
    QListWidgetItem *item_tmp = new QListWidgetItem;
    //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    item_tmp->setSizeHint(QSize(250,10));
    this->addItem(item_tmp);
    invalid_item->setObjectName("invalid_item");
    this->setItemWidget(item_tmp, invalid_item);
    item_tmp->setFlags(item_tmp->flags() & ~Qt::ItemIsSelectable);

    AddUserItem* addTip = new AddUserItem();
    QListWidgetItem* item = new QListWidgetItem;
    qDebug() << "addTip Size: " << addTip->sizeHint();
    item->setSizeHint(addTip->sizeHint());
    this->addItem(item);
    this->setItemWidget(item, addTip);
}

void SearchList::clearFindDialog()
{
    _find_dlg.reset();
}

void SearchList::slot_item_clicked(QListWidgetItem *item)
{
    QWidget* widget = this->itemWidget(item);
    if(widget == nullptr){
        qDebug() << "item widget is nullptr";
        return;
    }

    ListItemBase* itemBase = qobject_cast<ListItemBase*>(widget);
    if(itemBase == nullptr){
        qDebug() << "itemBase is nullptr";
        return;
    }

    if(itemBase->getItemType() == ListItemType::INVALID_ITEM){
        qDebug() << "itemBase type is invalid_item";
        return;
    }

    if(itemBase->getItemType() == ListItemType::ADD_USER_TIP_ITEM){
        // _find_dlg = std::make_shared<FindResultDialog>(itemBase);
        // auto si = std::make_shared<SearchInfo>(0, "hahah", "小美", "Hello", 0, "");
        // // dynamic_pointer_cast用于智能指针之间的转换
        // (std::dynamic_pointer_cast<FindResultDialog>(_find_dlg))->setSearchInfo(si);
        // _find_dlg->setSearchInfo(si);
        // _find_dlg->show();
        // return;

        auto* search_edit = dynamic_cast<CustomizeEdit*>(_search_edit);
        auto uid_str = search_edit->text();
        if(uid_str.isEmpty()){
            return;
        }

        QJsonObject root;
        root["uid"] = uid_str;
        QJsonDocument doc(root);
        QByteArray data = doc.toJson(QJsonDocument::Compact);
        emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_SEARCH_USER_REQ,
                                                  data);
        return;
    }

    clearFindDialog();
}

void SearchList::slot_search_user_success(std::shared_ptr<SearchInfo> si)
{
    auto iter_find = _handlers.find(ReqId::ID_SEARCH_USER_RSP);
    if(iter_find == _handlers.end()){
        qDebug() << "搜索用户信息回调函数未注册";
        return;
    }

    iter_find.value()(si);
}
