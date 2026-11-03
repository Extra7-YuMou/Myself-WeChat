#include "messagemodel.h"
#include "usermgr.h"
#include <QSize>

MessageModel::MessageModel(QObject *parent)
    : QAbstractListModel{parent}
{}

void MessageModel::appendMessage(const std::shared_ptr<ChatDataBase> msg)
{
    beginResetModel();
    _msg_list.append(msg);
    endResetModel();
}

void MessageModel::replaceOrAppend(const std::shared_ptr<ChatDataBase> msg)
{
    if(!msg){
        return;
    }

    for(int i = 0; i < _msg_list.size(); ++i){
        if(_msg_list[i] && _msg_list[i]->GetMsgId() == msg->GetMsgId()){
            _msg_list[i] = msg;
            emit dataChanged(index(i), index(i));
            return;
        }
    }

    appendMessage(msg);
}

void MessageModel::setHeadIcon()
{
    beginResetModel();
    endResetModel();
}

void MessageModel::setMessages(const QVector<std::shared_ptr<ChatDataBase> >& msg_list)
{
    beginResetModel();
    _msg_list = msg_list;
    endResetModel();
}

int MessageModel::rowCount(const QModelIndex &index) const
{
    return _msg_list.size();
}

QVariant MessageModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid() || index.row() < 0 || index.row() > _msg_list.size()){
        return QVariant();
    }

    auto msg = _msg_list[index.row()];
    if(!msg)return QVariant();

    switch(role){
    case RoleMessageId:
        return msg->GetMsgId();
    case RoleContent:
        return msg->GetContent();
    case RoleSendUid:
        return msg->GetSendUid();
    case RoleMsgType:
        return msg->GetMsgType();
    case RoleThreadId:
        return msg->GetThreadId();
    case RoleImgTotalPath: {
        auto img = std::dynamic_pointer_cast<ImgChatData>(msg);
        return img ? img->_total_path : QString();
    }
    case RolePixmapSize: {
        if(auto img = std::dynamic_pointer_cast<ImgChatData>(msg)){
            return img->_pix.isNull() ? QSize(120, 120) : img->_pix.size();
        }
        if(auto img = std::dynamic_pointer_cast<VideoChatData>(msg)){
            return img->_cover_pix.isNull() ? QSize(120, 120) : img->_cover_pix.size();
        }
        return QSize(120, 120);
    }
    case RoleVideoCoverPath: {
        auto v = std::dynamic_pointer_cast<VideoChatData>(msg);
        return v ? v->_cover_path : QString();
    }
    case RoleVideoDuration: {
        auto v = std::dynamic_pointer_cast<VideoChatData>(msg);
        return v ? v->_duration_sec : 0;
    }
    case RoleVideoPath : {
        auto v = std::dynamic_pointer_cast<VideoChatData>(msg);
        return v ? v->_video_path : QString();
    }
    default:
        break;
    }

    return QVariant();
}


