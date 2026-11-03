#ifndef USERDATA_H
#define USERDATA_H

#include <QString>
#include <QJsonArray>
#include <QJsonValueRef>
#include <QJsonObject>
#include "global.h"

class SearchInfo
{
public:
    SearchInfo(int uid, QString name, QString nick, QString desc, int sex, QString icon);
    int _uid;
    QString _name;
    QString _nick;
    QString _desc;
    int _sex;
    QString _icon;
};

class AddFriendApply{
public:
    AddFriendApply(int from_uid, QString name, QString desc, QString icon, QString nick, int sex);
    int _from_uid;
    QString _name;
    QString _desc;
    QString _icon;
    QString _nick;
    int _sex;
};

class ApplyInfo{
public:
    ApplyInfo(int uid, QString desc, QString icon, QString nick, int sex, int status);
    ApplyInfo(std::shared_ptr<AddFriendApply> addInfo);
    void setIcon(QString icon);

    int _uid;
    QString _desc;
    QString _nick;
    QString _icon;
    int _sex;
    int _status;
};

class AuthInfo{
public:
    AuthInfo(int uid, QString name, QString nick, QString icon, int sex);

    int _uid;
    QString _name;
    QString _nick;
    QString _icon;
    int _sex;
};

class AuthRsp{
public:
    AuthRsp(int uid, QString name, QString nick, QString icon, int sex);

    int _uid;
    QString _name;
    QString _nick;
    QString _icon;
    int _sex;
};

struct UserInfo {
    UserInfo(): _uid(0), _name(""), _nick(""), _icon(""), _sex(0), _desc(""), _back(""){}
    UserInfo(int uid, QString name, QString nick, QString icon, int sex, QString desc="", QString back=""):
        _uid(uid),_name(name),_nick(nick),_icon(icon),_sex(sex),_desc(desc), _back(back){}

    int _uid;
    QString _name;
    QString _nick;
    QString _icon;
    int _sex;
    QString _desc;
    QString _back;
};

struct ChatThreadInfo{
    ChatThreadInfo(qint64 thread_id, int user1_id, int user2_id):
        _thread_id(thread_id), _user1_id(user1_id), _user2_id(user2_id){}
    qint64 _thread_id;
    int _user1_id;
    int _user2_id;
};

class ChatDataBase {
public:
    ChatDataBase(qint64 msg_id, qint64 thread_id,  ChatMsgType msg_type,
                 QString content,int _send_uid);
    qint64 GetMsgId() { return _msg_id; }
    qint64 GetThreadId() { return _thread_id; }
    ChatMsgType GetMsgType() { return _msg_type; }
    QString GetContent() { return _content; }
    int GetSendUid() { return _send_uid; }
    QString GetMsgContent(){return _content;}
    void SetMsgType(ChatMsgType type) {_msg_type = type;}
    QString GetUniqueId();

    virtual ~ChatDataBase() = default;
private:
    //消息id
    qint64 _msg_id;
    //会话id
    qint64 _thread_id;
    //文本信息为1，图片为2，文件为3
    ChatMsgType _msg_type;
    QString _content;
    //发送者id
    int _send_uid;
};

class TextChatData : public ChatDataBase {
public:
    TextChatData(qint64 msg_id, qint64 thread_id, ChatMsgType msg_type,  QString content,
                 int send_uid):
        ChatDataBase(msg_id, thread_id, msg_type, content, send_uid){}
};

class ImgChatData : public ChatDataBase{
public:
    ImgChatData(qint64 msg_id, qint64 thread_id, ChatMsgType msg_type, QString name, int send_uid, QString total_path):
        ChatDataBase(msg_id, thread_id, msg_type, name, send_uid), _total_path(total_path){
        _pix = QPixmap(total_path);

        const int MAX_LENGTH = 200;
        const int MIN_LENGTH = 120;

        if(_pix.width() > _pix.height()){
            _pix = _pix.scaled(QSize(MAX_LENGTH, MIN_LENGTH), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }else if(_pix.width() == _pix.height()){
            _pix = _pix.scaled(QSize(MAX_LENGTH, MAX_LENGTH), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }else{
            _pix = _pix.scaled(QSize(MIN_LENGTH, MAX_LENGTH), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
    }

    QString _total_path;
    QPixmap _pix;
};

class VideoChatData : public ChatDataBase{
public:
    VideoChatData(qint64 msg_id, qint64 thread_id, ChatMsgType msg_type, QString name, int send_uid,
                  QString video_path, QString cover_path, int duration_sec):
        ChatDataBase(msg_id, thread_id, msg_type, name, send_uid),
        _video_path(video_path), _cover_path(cover_path), _duration_sec(duration_sec)
    {
        _cover_pix = QPixmap(cover_path);
        if(!_cover_pix.isNull()){
            _cover_pix = _cover_pix.scaled(QSize(200, 120),
                                           Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
    }

    QString _video_path;
    QString _cover_path;
    QPixmap _cover_pix;
    int _duration_sec = 0;
};

//客户端本地存储的聊天线程数据结构
class ChatThreadData {
public:
    ChatThreadData(qint64 thread_id, qint64 last_message_id, int other_uid);
    qint64 GetThreadId();
    qint64 GetLastMessageId();
    void AddMsg(qint64 message_id, std::shared_ptr<ChatDataBase> data);
    int& GetOtherUid();
    std::shared_ptr<ChatDataBase> GetChatData(qint64 message_id);
    QMap<qint64, std::shared_ptr<ChatDataBase>>& GetChatData();
private:
    int _other_uid;
    qint64 _last_message_id;
    qint64 _thread_id;
    QMap<qint64, std::shared_ptr<ChatDataBase>> _chat_data;
};

// 文件信息
class FileInfo{
public:
    FileInfo(const QString& name = "", const int& seq = 0, const int& trans_size = 0,
             const qint64& total_size = 0): _name(name), _seq(seq), _trans_size(trans_size),
        _total_size(total_size){}

    QString _name;
    int _seq;
    int _trans_size;
    qint64 _total_size;
};

#endif // USERDATA_H
