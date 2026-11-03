#include "chatuseritem.h"
#include "ui_chatuseritem.h"
#include <QDir>
#include "usermgr.h"

ChatUserItem::ChatUserItem(QWidget *parent) :
    ListItemBase(parent), ui(new Ui::ChatUserItem), _UnReadCount(0), _chat_data(nullptr)
{
    ui->setupUi(this);
    setItemType(ListItemType::CHAT_USER_ITEM);
}

ChatUserItem::~ChatUserItem()
{
    delete ui;
}

void ChatUserItem::setInfo(std::shared_ptr<ChatThreadData> chat_data)
{
    if(chat_data == nullptr){
        return;
    }
    _chat_data = chat_data;
    auto user_info = UserMgr::GetInstance()->GetFriendList()[chat_data->GetOtherUid()];
    if(user_info == nullptr){
        qDebug() << "user_info is nullptr";
        return;
    }
    QString appPath = QCoreApplication::applicationDirPath();
    QString iconPath = GetDownLoadFilePrefix() + QDir::separator() + "avatars" + QDir::separator() + user_info->_icon;
    QPixmap icon(iconPath);
    ui->icon_lb->setPixmap(icon.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);

    ui->nick_lb->setText(user_info->_nick);

    QString currentTime = QTime::currentTime().toString("HH:mm");
    ui->time_lb->setText(currentTime);
}

QString ChatUserItem::getNick()
{
    return ui->nick_lb->text();
}

void ChatUserItem::setUnReadCount(int UnReadCount)
{
    ui->icon_lb->setUnReadCount(UnReadCount);
}

void ChatUserItem::setLastMsg(const QString &msg)
{
    _UnReadCount++;
    setUnReadCount(_UnReadCount);
    ui->msg_lb->setText(msg);
}

void ChatUserItem::SetChatData(std::shared_ptr<ChatThreadData> chat_data)
{
    if(chat_data == nullptr){
        qDebug() << "传入了空的chat_data";
        return;
    }

    _chat_data = chat_data;
}

void ChatUserItem::AddChatMessage(QVector<std::shared_ptr<ChatDataBase> > data_list)
{
    if(data_list.isEmpty()){
        return;
    }

    auto& thread_data = _chat_data->GetChatData();
    for(auto& data : data_list){
        thread_data[data->GetMsgId()] = data;
    }
}

std::shared_ptr<ChatThreadData> ChatUserItem::GetChatData()
{
    return _chat_data;
}

void ChatUserItem::mousePressEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton && e->type() == QMouseEvent::MouseButtonPress){
        emit sig_loading_data(UserMgr::GetInstance()->GetFriendList()[_chat_data->GetOtherUid()], _chat_data);
    }
    QWidget::mousePressEvent(e);
}
