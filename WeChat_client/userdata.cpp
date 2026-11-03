#include "userdata.h"

SearchInfo::SearchInfo(int uid, QString name, QString nick, QString desc, int sex, QString icon):
    _uid(uid), _name(name), _nick(nick), _desc(desc), _sex(sex), _icon(icon)
{}

AddFriendApply::AddFriendApply(int from_uid, QString name, QString desc,
                               QString icon, QString nick, int sex):
    _from_uid(from_uid), _name(name), _desc(desc), _icon(icon), _nick(nick), _sex(sex)
{}

ApplyInfo::ApplyInfo(int uid, QString desc, QString icon, QString nick, int sex, int status):
    _uid(uid), _desc(desc), _icon(icon), _nick(nick), _sex(sex), _status(status)
{}

ApplyInfo::ApplyInfo(std::shared_ptr<AddFriendApply> addInfo):
    _uid(addInfo->_from_uid), _nick(addInfo->_nick), _icon(addInfo->_icon),
    _sex(addInfo->_sex), _status(0)
{ }

void ApplyInfo::setIcon(QString icon)
{
    _icon = icon;
}

AuthInfo::AuthInfo(int uid, QString name, QString nick, QString icon, int sex):
    _uid(uid), _name(name), _nick(nick), _icon(icon), _sex(sex)
{ }

AuthRsp::AuthRsp(int uid, QString name, QString nick, QString icon, int sex):
    _uid(uid), _name(name), _nick(nick), _icon(icon), _sex(sex)
{}

ChatDataBase::ChatDataBase(qint64 msg_id, qint64 thread_id, ChatMsgType msg_type, QString content, int send_uid):
    _msg_id(msg_id), _thread_id(thread_id), _msg_type(msg_type), _content(content), _send_uid(send_uid){}

ChatThreadData::ChatThreadData(qint64 thread_id, qint64 last_message_id, int other_uid):
    _thread_id(thread_id), _last_message_id(last_message_id), _other_uid(other_uid)
{    }

qint64 ChatThreadData::GetThreadId()
{
    return _thread_id;
}

qint64 ChatThreadData::GetLastMessageId()
{
    return _last_message_id;
}

void ChatThreadData::AddMsg(qint64 message_id, std::shared_ptr<ChatDataBase> data)
{
    if(data == nullptr){
        return;
    }

    if(_chat_data[message_id] != nullptr){
        return;
    }

    _last_message_id = _last_message_id > message_id ? _last_message_id : message_id;
    _chat_data[message_id] = data;
}

int& ChatThreadData::GetOtherUid()
{
    return _other_uid;
}

std::shared_ptr<ChatDataBase> ChatThreadData::GetChatData(qint64 message_id)
{
    if(_chat_data[message_id] == nullptr){
        return nullptr;
    }
    return _chat_data[message_id];
}

QMap<qint64, std::shared_ptr<ChatDataBase> > &ChatThreadData::GetChatData()
{
    return _chat_data;
}
