#include "tcpmgr.h"
#include <QAbstractSocket>
#include <QString>
#include "usermgr.h"
#include <QMessageBox>
#include "sqlmgr.h"
#include <QDir>

TcpMgr::TcpMgr() : _host(""), _port(0), _b_recv_head(false), _message_id(0), _message_len(0){
    connect(&_sock, &QTcpSocket::connected, [this](){
        qDebug() << "Connect ChatServer has successed";
        emit sig_con_success();
    });

    connect(&_sock, &QTcpSocket::readyRead, [this](){
        _buffer.append(_sock.readAll());

        QDataStream ds(&_buffer, QIODevice::ReadOnly);
        ds.setVersion(QDataStream::Qt_6_0);

        forever{
            if(!_b_recv_head){
                if(_buffer.size() < TCP_HEAD_LEN){
                    return;
                }

                ds >> _message_id >> _message_len;

                _buffer = _buffer.mid(TCP_HEAD_LEN);
                qDebug() << "Receive message id is " << _message_id
                         << ", data len is " << _message_len;
            }

            if(_buffer.size() < _message_len){
                _b_recv_head = true;
                return;
            }

            _b_recv_head = false;
            QByteArray _message_body = _buffer.mid(0, _message_len);
            _buffer = _buffer.mid(_message_len);

            qDebug() << "Receive message body is " << _message_body;
            handleMsg(_message_id, _message_len, _message_body);
        }
    });

    //5.15 之后版本
    QObject::connect(&_sock, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred), [&](QAbstractSocket::SocketError socketError) {
        Q_UNUSED(socketError)
        qDebug() << "Error:" << _sock.errorString();
    });

    connect(&_sock, &QTcpSocket::disconnected, [this](){
        qDebug() << "Server has not connect";
    });

    connect(this, &TcpMgr::sig_send_data, this, &TcpMgr::slot_send_data);

    initHandlers();
}

void TcpMgr::initHandlers(){
    _handlers.insert(ReqId::ID_CHAT_LOGIN_RSP, [this](TCP_ID_LEN id, TCP_BODY_LEN len, QByteArray data){
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if(doc.isNull()){
            qDebug() << "json 数据为空";
            emit sig_login_failed(ErrorCodes::ERR_NETWORK);
            return;
        }

        QJsonObject json = doc.object();
        if(!json.contains("error")){
            qDebug() << "login failed, err is json parse err";
            emit sig_login_failed(ErrorCodes::ERR_JSON);
            return;
        }

        int err = json["error"].toInt();
        if(err != ErrorCodes::SUCCESS){
            qDebug() << "Login failed, err is " << err;
            emit sig_login_failed(err);
            return;
        }

        // user管理类操作
        UserMgr::GetInstance()->setName(json["name"].toString());
        UserMgr::GetInstance()->setNick(json["nick"].toString());
        UserMgr::GetInstance()->setUid(json["uid"].toInt());
        UserMgr::GetInstance()->setIcon(json["icon"].toString());
        UserMgr::GetInstance()->setDesc(json["desc"].toString());

        // 获取好友申请列表
        std::vector<std::shared_ptr<ApplyInfo>> apply_info;
        if(json.contains("apply_list")){
            QJsonArray apply_list = json.value("apply_list").toArray();
            for(const QJsonValue& obj : apply_list){
                if(!obj.isObject()){
                    qDebug() << "提取好友申请信息错误";
                    continue;
                }

                QJsonObject root = obj.toObject();
                ApplyInfo info(root["from_uid"].toInt(),
                               root["desc"].toString(),
                               root["icon"].toString(),
                               root["nick"].toString(),
                               root["sex"].toInt(),
                               root["status"].toInt());
                apply_info.push_back(std::make_shared<ApplyInfo>(info));
            }
        }

        // 获取好友列表
        QMap<int, std::shared_ptr<UserInfo>> user_list;
        if(json.contains("friend_list")){
            QJsonArray friend_list = json.value("friend_list").toArray();
            for(const QJsonValue& info : friend_list){
                if(!info.isObject()){
                    qDebug() << "提取好友列表信息错误";
                    continue;
                }

                QJsonObject root = info.toObject();
                auto user_info = std::make_shared<UserInfo>(root["uid"].toInt(), root["name"].toString(),
                                                            root["nick"].toString(), root["icon"].toString(),
                                                            root["sex"].toInt());
                user_list.insert(root["uid"].toInt(), user_info);
                qDebug() << "friend uid: " << user_info->_uid;
                SqlMgr::GetInstance()->InsertUserInfo(user_info);
            }
        }

        QMap<qint64, int> unread_map;
        if(json.contains("unread_thread")){
            QJsonArray _map = json.value("unread_thread").toArray();
            for(const QJsonValue& info : _map){
                if(!info.isObject()){
                    qDebug() << "提取会话未读信息数量错误";
                    continue;
                }

                QJsonObject root = info.toObject();
                for(auto it = root.begin(); it != root.end(); ++it){
                    qDebug() << "t id: " << it.key().toLongLong() << ", unread count: " << it.value().toInt();
                    unread_map[it.key().toLongLong()] = it.value().toInt();
                }
            }
        }

        qint64 unread_count = json["unread_msg_count"].toInteger();

        emit switchChat(apply_info, user_list, unread_count, unread_map);
    });

    _handlers[ReqId::ID_SEARCH_USER_RSP] = [this](TCP_ID_LEN id, TCP_BODY_LEN len, QByteArray data){
        qDebug() << "Handle id is " << id << ", data is " << data;
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if(doc.isNull()){
            qDebug() << "Json 解析失败";
            int err = ErrorCodes::ERR_JSON;
            emit sig_search_user_failed(err);
            return;
        }

        QJsonObject json = doc.object();
        if(!json.contains("error")){
            qDebug() << "Json 解析失败";
            int err = ErrorCodes::ERR_JSON;
            emit sig_search_user_failed(err);
            return;
        }

        std::shared_ptr<SearchInfo> si = std::make_shared<SearchInfo>(json["uid"].toInt(),
                                                                       json["name"].toString(),
                                                                       json["nick"].toString(),
                                                                       json["desc"].toString(),
                                                                       json["sex"].toInt(),
                                                                       json["icon"].toString());
        emit sig_search_user_success(si);
    };

    _handlers[ID_APPLY_FRIEND_RSP] = [this](TCP_ID_LEN id, TCP_BODY_LEN len, QByteArray data){
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if(doc.isNull()){
            qDebug() << "Json解析失败";
            emit sig_apply_friend_failed(ErrorCodes::ERR_JSON);
            return;
        }

        QJsonObject json = doc.object();
        if(!json.contains("error")){
            qDebug() << "json接收数据不全";
            emit sig_apply_friend_failed(ErrorCodes::ERR_JSON);
            return;
        }

        if(json["error"].toInt() != ErrorCodes::SUCCESS){
            qDebug() << "添加好友失败";
            emit sig_apply_friend_failed(json["error"].toInt());
            return;
        }

        std::shared_ptr<AddFriendApply> ai(new AddFriendApply(json["from_uid"].toInt(),
                                                      json["name"].toString(),
                                                      json["desc"].toString(),
                                                      json["icon"].toString(),
                                                      json["nick"].toString(),
                                                      json["sex"].toInt()));
        emit sig_apply_friend_success(ai);
    };

    _handlers[ReqId::ID_AUTH_FRIEND_RSP] = [this](TCP_ID_LEN id, TCP_BODY_LEN len, QByteArray data){
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if(doc.isNull()){
            qDebug() << "添加好友回包json解析错误";
            return;
        }

        QJsonObject json = doc.object();
        if(!json.contains("error")){
            qDebug() << "添加好友回包json解析错误";
            return;
        }

        int err = json["error"].toInt();
        if(err != ErrorCodes::SUCCESS){
            qDebug() << "添加好友回包出现错误，错误码: " << err;
            return;
        }

        auto uid = json["uid"].toInt();
        auto name = json["name"].toString();
        auto nick = json["nick"].toString();
        auto icon = json["icon"].toString();
        auto sex = json["sex"].toInt();
        auto thread_id = json["thread_id"].toInteger();
        auto message_id = json["message_id"].toInteger();
        auto friend_msg = json["friend_msg"].toString();
        auto self_msg = json["self_msg"].toString();
        auto user_info = std::make_shared<UserInfo>(uid, name, nick, icon, sex);
        auto thread_data = std::make_shared<ChatThreadData>(thread_id, message_id, uid);
        auto self_uid = UserMgr::GetInstance()->getUid();
        auto friend_data_base = std::make_shared<ChatDataBase>(message_id - 1, thread_id, ChatMsgType::Text, friend_msg, uid);
        auto self_data_base = std::make_shared<ChatDataBase>(message_id, thread_id, ChatMsgType::Text, self_msg, self_uid);

        UserMgr::GetInstance()->GetFriendList()[uid] = user_info;

        thread_data->AddMsg(message_id - 1, friend_data_base);
        thread_data->AddMsg(message_id, self_data_base);

        if(!SqlMgr::GetInstance()->InsertUserInfo(user_info)){
            //return;
            qDebug() << "插入好友信息失败,uid: " << user_info->_uid;
        }
        if(!SqlMgr::GetInstance()->InsertChatMessage(thread_data->GetChatData(message_id - 1), uid)){
            // return;
            qDebug() << "插入消息失败,message id: " << message_id - 1;
        }
        if(!SqlMgr::GetInstance()->InsertChatMessage(thread_data->GetChatData(message_id), self_uid)){
            // return;
            qDebug() << "插入消息失败,message id: " << message_id;
        }

        // 用于通知被申请方添加好友成功
        emit sig_auth_friend_success(thread_data);
    };

    _handlers[ReqId::ID_NOTIFY_FRIEND_RSP] = [this](TCP_ID_LEN id, TCP_BODY_LEN len, QByteArray data){
        qDebug() << "执行通知好友申请通过回调";
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if(doc.isNull()){
            qDebug() << "通知好友申请通过回包json解析错误";
            return;
        }

        QJsonObject json = doc.object();
        if(!json.contains("error")){
            qDebug() << "通知好友申请通过回包json解析错误";
            return;
        }

        int err = json["error"].toInt();
        if(err != ErrorCodes::SUCCESS){
            qDebug() << "通知好友申请通过回包出现错误，错误码: " << err;
            return;
        }

        auto uid = json["uid"].toInt();
        auto name = json["name"].toString();
        auto nick = json["nick"].toString();
        auto icon = json["icon"].toString();
        auto sex = json["sex"].toInt();
        auto thread_id = json["thread_id"].toInteger();
        auto message_id = json["message_id"].toInteger();
        auto friend_msg = json["friend_msg"].toString();
        auto self_msg = json["self_msg"].toString();
        auto user_info = std::make_shared<UserInfo>(uid, name, nick, icon, sex);
        auto thread_data = std::make_shared<ChatThreadData>(thread_id, message_id, uid);
        auto self_uid = UserMgr::GetInstance()->getUid();
        auto friend_data_base = std::make_shared<ChatDataBase>(message_id - 1, thread_id, ChatMsgType::Text, friend_msg, uid);
        auto self_data_base = std::make_shared<ChatDataBase>(message_id, thread_id, ChatMsgType::Text, self_msg, self_uid);

        UserMgr::GetInstance()->GetFriendList()[uid] = user_info;

        thread_data->AddMsg(message_id - 1, friend_data_base);
        thread_data->AddMsg(message_id, self_data_base);

        if(!SqlMgr::GetInstance()->InsertUserInfo(user_info)){
            //return;
            qDebug() << "插入好友信息失败,uid: " << user_info->_uid;
        }
        if(!SqlMgr::GetInstance()->InsertChatMessage(thread_data->GetChatData(message_id - 1), uid)){
            // return;
            qDebug() << "插入消息失败,message id: " << message_id - 1;
        }
        if(!SqlMgr::GetInstance()->InsertChatMessage(thread_data->GetChatData(message_id), self_uid)){
            // return;
            qDebug() << "插入消息失败,message id: " << message_id;
        }

        emit sig_notify_friend_success(thread_data);
    };

    _handlers[ReqId::ID_TEXT_CHAT_MSG_RSP] = [this](TCP_ID_LEN id, TCP_BODY_LEN len, QByteArray data){
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if(doc.isNull()){
            qDebug() << "消息发送json解析错误";
            return;
        }

        QJsonObject json = doc.object();
        if(!json.contains("error")){
            qDebug() << "消息发送json解析错误";
            return;
        }

        int err = json["error"].toInt();
        if(err != ErrorCodes::SUCCESS){
            qDebug() << "消息发送失败";
            return;
        }

        auto friend_uid = json["friend_uid"].toInt();
        auto self_uid = json["self_uid"].toInt();
        auto thread_id = json["thread_id"].toInteger();
        QVector<std::shared_ptr<ChatDataBase>> msg_vec;
        if(json.contains("text_array")){
            QJsonArray array = json["text_array"].toArray();
            for(const QJsonValue& value : array){
                if(!value.isObject()){
                    qDebug() << "提取消息失败";
                    continue;
                }

                QJsonObject root = value.toObject();
                auto content = root["content"].toString();
                auto msg_id = root["message_id"].toInteger();
                auto type = root["type"].toInt();

                auto chat_msg = std::make_shared<ChatDataBase>(msg_id, thread_id,
                                                               ChatMsgType(type), content, self_uid);

                if(!SqlMgr::GetInstance()->InsertChatMessage(chat_msg, friend_uid)){
                    qDebug() << "插入聊天消息失败";
                }
                msg_vec.push_back(chat_msg);
            }
        }

        emit sig_text_chat_msg(msg_vec, friend_uid);
    };

    _handlers[ReqId::ID_NOTIFY_TEXT_CHAT_MSG_RSP] = [this](TCP_ID_LEN id, TCP_BODY_LEN len, QByteArray data){
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if(doc.isNull()){
            qDebug() << "接收聊天消息Json解析错误";
            return;
        }

        QJsonObject json = doc.object();
        if(!json.contains("error")){
            qDebug() << "接受聊天消息Json解析错误";
            return;
        }

        int err = json["error"].toInt();
        if(err != ErrorCodes::SUCCESS){
            qDebug() << "接受聊天消息Json解析错误";
            return;
        }

        auto friend_uid = json["friend_uid"].toInt();
        auto self_uid = json["self_uid"].toInt();
        auto thread_id = json["thread_id"].toInteger();
        qDebug() << "f uid: " << friend_uid << ", s uid: " << self_uid;
        QVector<std::shared_ptr<ChatDataBase>> msg_vec;
        if(json.contains("text_array")){
            QJsonArray array = json["text_array"].toArray();
            for(const QJsonValue& value : array){
                if(!value.isObject()){
                    qDebug() << "提取消息失败";
                    continue;
                }

                QJsonObject root = value.toObject();
                auto content = root["content"].toString();
                auto msg_id = root["message_id"].toInteger();
                auto type = root["type"].toInt();

                auto chat_msg = std::make_shared<ChatDataBase>(msg_id, thread_id,
                                                               ChatMsgType(type), content, friend_uid);

                if(!SqlMgr::GetInstance()->InsertChatMessage(chat_msg, self_uid)){
                    qDebug() << "插入聊天消息失败";
                }
                msg_vec.push_back(chat_msg);
            }
        }

        emit sig_notify_text_chat_msg(msg_vec, friend_uid);
    };

    _handlers[ReqId::ID_NOTIFY_OFF_LINE_RSP] = [this](TCP_ID_LEN id, TCP_BODY_LEN len, QByteArray data){
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if(doc.isNull()){
            qDebug() << "接收下线通知Json解析错误";
            return;
        }

        QJsonObject json = doc.object();
        if(!json.contains("error")){
            qDebug() << "接受下线通知Json解析错误";
            return;
        }

        int err = json["error"].toInt();
        if(err != ErrorCodes::SUCCESS){
            qDebug() << "接受下线消息Json解析错误, Err: " << err;
            return;
        }

        int uid = json["uid"].toInt();
        emit sig_notify_off_line(uid);
    };

    _handlers[ID_HEARTBEAT_RSP] = [this](TCP_ID_LEN id, TCP_BODY_LEN len, QByteArray data){
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if(doc.isNull()){
            qDebug() << "定时心跳检测Json解析错误";
            return;
        }

        QJsonObject json = doc.object();
        if(!json.contains("error")){
            qDebug() << "定时心跳检测json数据格式错误";
            return;
        }

        int err = json["error"].toInt();
        if(err != ErrorCodes::SUCCESS){
            qDebug() << "定时心跳检测出现错误，错误码: " << err;
            return;
        }

        qDebug() << "uid: " << json["uid"].toInt() << "的用户活着";
    };

    _handlers[ID_UPDATE_USERINFO_RSP] = [this](TCP_ID_LEN id, TCP_BODY_LEN len, QByteArray data){
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if(doc.isNull()){
            qDebug() << "更新用户信息Json解析错误";
            return;
        }

        QJsonObject json = doc.object();
        if(!json.contains("error")){
            qDebug() << "更新用户信息json数据格式错误";
            return;
        }

        int err = json["error"].toInt();
        if(err != ErrorCodes::SUCCESS){
            qDebug() << "更新用户信息出现错误，错误码: " << err;
            return;
        }

        UserMgr::GetInstance()->setNick(json["nikc"].toString());
        UserMgr::GetInstance()->setDesc(json["desc"].toString());
        QMessageBox::information(nullptr, "更新成功", "用户信息更新成功");
    };

    _handlers[ID_LOAD_CHAT_THREAD_RSP] = [this](TCP_ID_LEN id, TCP_BODY_LEN len, QByteArray data){
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if(doc.isNull()){
            qDebug() << "获取聊天记录Json解析错误";
            return;
        }

        QJsonObject json = doc.object();
        if(!json.contains("error")){
            qDebug() << "获取聊天记录json数据格式错误";
            return;
        }

        int err = json["error"].toInt();
        if(err != ErrorCodes::SUCCESS){
            qDebug() << "获取聊天记录出现错误，错误码: " << err;
            return;
        }

        QVector<std::shared_ptr<ChatThreadData>> thread_list;
        if(json.contains("chat_thread")){
            if(json["load_count"].toInt() == 0){
                qDebug() << "没有获取的聊天会话";
                emit sig_close_tip_wid();
                return;
            }
            QJsonArray array = json["chat_thread"].toArray();
            for(const QJsonValue& obj : array){
                if(!obj.isObject())   {
                    qDebug() << "提取聊天记录错误";
                    continue;
                }

                QJsonObject root = obj.toObject();
                UserInfo user_info(root["other_uid"].toInt(),
                                   root["other_name"].toString(),
                                   root["other_nick"].toString(),
                                   root["other_icon"].toString(),
                                   root["other_sex"].toInt());
                qDebug() << "id: " << user_info._uid << ", name: " << user_info._name
                         << ", nick: " << user_info._nick;
                auto chat_info = std::make_shared<ChatThreadInfo>(
                    root["thread_id"].toInteger(),
                    root["user1_id"].toInt(),
                    root["user2_id"].toInt());
                UserMgr::GetInstance()->addChatThread(chat_info);
                bool res = SqlMgr::GetInstance()->InsertUserInfo(std::make_shared<UserInfo>(user_info));
                if(!res){
                    return;
                }
                res = SqlMgr::GetInstance()->InsertChatThread(chat_info);
                if(!res){
                    return;
                }
            }
        }else{
            emit sig_close_tip_wid();
            return;
        }

        // 发送信号通知ChatDialog加载ChatThread
        qDebug() << "load chat thread";
        emit sig_load_chat_thread();
    };

    _handlers[ID_LOAD_CHAT_MESSAGE_RSP] = [this](TCP_ID_LEN id, TCP_BODY_LEN len, QByteArray data){
        emit sig_close_tip_wid();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if(doc.isNull()){
            qDebug() << "获取聊天消息Json解析错误";
            return;
        }

        QJsonObject json = doc.object();
        if(!json.contains("error")){
            qDebug() << "获取聊天消息json数据格式错误";
            return;
        }

        int err = json["error"].toInt();
        if(err != ErrorCodes::SUCCESS){
            qDebug() << "获取聊天消息出现错误，错误码: " << err;
            return;
        }

        QVector<std::shared_ptr<ChatDataBase>> _data_list;
        if(json.contains("chat_message")){
            if(json["load_count"].toInt() == 0){
                emit sig_close_tip_wid();
                return;
            }
            QJsonArray array = json["chat_message"].toArray();
            for(const QJsonValue& obj : array){
                if(!obj.isObject()){
                    qDebug() << "提取聊天消息失败";
                    continue;
                }

                QJsonObject root = obj.toObject();
                auto message_id = root["message_id"].toInteger();
                auto thread_id = root["thread_id"].toInteger();
                auto sender_id = root["sender_id"].toInt();
                auto recv_id = root["recv_id"].toInt();
                auto type = root["type"].toInt();
                auto content = root["content"].toString();
                auto status = root["status"].toInt();

                std::shared_ptr<ChatDataBase> chat_data;
                if(type == ChatMsgType::Img){
                    QString local_path = GetLocalFilePrefix(thread_id) + content;
                    chat_data = std::make_shared<ImgChatData>(
                        message_id, thread_id, ChatMsgType::Img, content, sender_id, local_path);
                }else if(type == ChatMsgType::Video){
                    QString videoPath = GetLocalFilePrefix(thread_id) + content;
                    QString coverName = content;
                    coverName.replace(QRegularExpression("\\.[^.]+$"), "_cover.jpg");
                    QString coverPath = GetDownLoadFilePrefix() + QDir::separator() + "video_covers" + QDir::separator() + coverName;
                    chat_data = std::make_shared<VideoChatData>(message_id, thread_id, ChatMsgType::Video, content, sender_id,
                        videoPath, coverPath, 0);
                }else{
                    chat_data = std::make_shared<ChatDataBase>(
                        message_id, thread_id, ChatMsgType(type), content, sender_id);
                }

                if(!SqlMgr::GetInstance()->InsertChatMessage(chat_data, recv_id)){
                    qDebug() << "插入聊天消息失败";
                    emit sig_close_tip_wid();
                    return;
                }

                _data_list.push_back(chat_data);
            }
        }
        qDebug() << "load chat message";
        emit sig_load_chat_message(_data_list);
    };

    _handlers[ID_IMG_CHAT_MSG_RSP] = [this](TCP_ID_LEN id, TCP_BODY_LEN len, QByteArray msg_data){
        QJsonDocument doc = QJsonDocument::fromJson(msg_data);
        if(doc.isNull()){
            qDebug() << "获取聊天图片消息Json解析错误";
            return;
        }

        QJsonObject json = doc.object();
        if(!json.contains("error")){
            qDebug() << "获取聊天图片消息json数据格式错误";
            return;
        }

        int err = json["error"].toInt();
        if(err != ErrorCodes::SUCCESS){
            qDebug() << "获取聊天图片消息出现错误，错误码: " << err;
            return;
        }

        auto friend_uid = json["friend_uid"].toInt();
        auto self_uid = json["self_uid"].toInt();
        auto thread_id = json["thread_id"].toInteger();
        auto name = json["name"].toString();
        auto type = json["type"].toInt();
        auto message_id = json["message_id"].toInteger();
        auto path = json["path"].toString();

        auto img_msg = std::make_shared<ImgChatData>(
            message_id, thread_id, ChatMsgType(type), name, self_uid, path);
        emit sig_upload_img_chat_message(img_msg);
    };

    _handlers[ID_NOTIFY_UPLOAD_IMG_CHAT_RSP] = [this](TCP_ID_LEN id, TCP_BODY_LEN len, QByteArray msg_data){
        QJsonDocument doc = QJsonDocument::fromJson(msg_data);
        if(doc.isNull()){
            qDebug() << "获取聊天图片上传完成消息Json解析错误";
            return;
        }

        QJsonObject json = doc.object();
        if(!json.contains("error")){
            qDebug() << "获取聊天图片上传完成消息json数据格式错误";
            return;
        }

        int err = json["error"].toInt();
        if(err != ErrorCodes::SUCCESS){
            qDebug() << "获取聊天图片上传完成消息出现错误，错误码: " << err;
            return;
        }

        auto msg_id = json["message_id"].toInteger();
        auto name = json["name"].toString();
        auto thread_id = json["thread_id"].toInteger();
        auto send_uid = json["send_uid"].toInt();
        auto type = json["type"].toInt();
        auto md5 = json["md5"].toString();

        auto img_data = std::make_shared<ChatDataBase>(msg_id, thread_id, ChatMsgType::Img, name, send_uid);
        if(!SqlMgr::GetInstance()->InsertChatMessage(img_data, 0)){
            qDebug() << "插入消息失败, 消息内容: " << name << ", 会话id: " << thread_id << ", 类型: " << type;
            return;
        }

        emit sig_download_file(msg_id, thread_id, md5, name, send_uid);
    };

    _handlers[ID_NOTIFY_DOWNLOAD_IMG_CHAT_RSP] = [this](TCP_ID_LEN id, TCP_BODY_LEN len, QByteArray msg_data){
        QJsonDocument doc = QJsonDocument::fromJson(msg_data);
        if(doc.isNull()){
            qDebug() << "通知下载聊天图片Json解析错误";
            return;
        }

        QJsonObject json = doc.object();
        if(!json.contains("error")){
            qDebug() << "通知下载聊天图片Json解析错误";
            return;
        }

        int err = json["error"].toInt();
        if(err != ErrorCodes::SUCCESS){
            qDebug() << "通知下载聊天图片出现错误, 错误码: " << err;
            return;
        }

        auto md5 = json["md5"].toString();
        auto name = json["name"].toString();
        auto send_uid = json["send_uid"].toInt();
        auto recv_uid = json["recv_uid"].toInt();
        auto msg_id = json["message_id"].toInteger();
        auto thread_id = json["thread_id"].toInteger();
        auto total_size = json["total_size"].toInteger();

        auto img_data = std::make_shared<ChatDataBase>(msg_id, thread_id, ChatMsgType::Img, name, send_uid);
        if(!SqlMgr::GetInstance()->InsertChatMessage(img_data, recv_uid)){
            qDebug() << "插入消息失败";
            return;
        }

        emit sig_notify_download_file(send_uid, thread_id, msg_id, name, md5);
    };

    _handlers[ReqId::ID_VIDEO_CHAT_MSG_RSP] = [this](TCP_ID_LEN id, TCP_BODY_LEN len, QByteArray data){
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if(doc.isNull()){
            qDebug() << "发送视频消息失败";
            return;
        }

        QJsonObject json = doc.object();
        if(!json.contains("error")){
            qDebug() << "发送视频消息回包错误";
            return;
        }

        int err = json["error"].toInt();
        if(err != ErrorCodes::SUCCESS){
            qDebug() << "发送视频消息回包参数错误， error: " << err;
            return;
        }

        auto self_uid = json["self_uid"].toInt();
        auto friend_uid = json["friend_uid"].toInt();
        auto thread_id = json["thread_id"].toInteger();
        auto name = json["name"].toString();
        auto coverName = json["cover_name"].toString();
        auto path = json["path"].toString();
        auto coverPath = json["cover_path"].toString();
        auto duration = json["duration"].toInt();
        auto type = json["type"].toInt();
        auto msg_id = json["message_id"].toInteger();

        auto videoChatData = std::make_shared<VideoChatData>(msg_id, thread_id,
                                                             ChatMsgType(type), name, self_uid, path, coverPath, duration);
        if(!SqlMgr::GetInstance()->InsertChatMessage(videoChatData, friend_uid)){
            qDebug() << "插入消息失败";
            return;
        }
        emit sig_upload_video_chat_msg(videoChatData);
    };

    _handlers[ID_NOTIFY_DOWNLOAD_VIDEO_CHAT_MSG_RSP] = [this](TCP_ID_LEN id, TCP_BODY_LEN len, QByteArray msg_data){
        QJsonDocument doc = QJsonDocument::fromJson(msg_data);
        if(doc.isNull()){
            qDebug() << "通知下载聊天图片Json解析错误";
            return;
        }

        QJsonObject json = doc.object();
        if(!json.contains("error")){
            qDebug() << "通知下载聊天图片Json解析错误";
            return;
        }

        int err = json["error"].toInt();
        if(err != ErrorCodes::SUCCESS){
            qDebug() << "通知下载聊天图片出现错误, 错误码: " << err;
            return;
        }

        auto md5 = json["md5"].toString();
        auto name = json["name"].toString();
        auto send_uid = json["send_uid"].toInt();
        auto recv_uid = json["recv_uid"].toInt();
        auto msg_id = json["message_id"].toInteger();
        auto thread_id = json["thread_id"].toInteger();
        auto coverName = json["cover_name"].toString();
        auto duration = json["duration"].toInt();

        auto video_data = std::make_shared<ChatDataBase>(msg_id, thread_id, ChatMsgType::Video, name, send_uid);
        if(!SqlMgr::GetInstance()->InsertChatMessage(video_data, recv_uid)){
            qDebug() << "插入消息失败";
            return;
        }

        emit sig_download_video_chat_msg(send_uid, thread_id, msg_id, name, coverName, duration);
    };
}

void TcpMgr::handleMsg(int id, int len, QByteArray data){
    auto iter = _handlers.find(id);
    if(iter == _handlers.end()){
        qDebug() << "未找到对应tcp回调函数";
        return;
    }
    iter.value()(id, len, data);
}

void TcpMgr::slot_send_data(ReqId reqid, QString data)
{
    uint16_t id = reqid;
    QByteArray dataByte = data.toUtf8();
    TCP_BODY_LEN len = static_cast<TCP_BODY_LEN>(dataByte.size());

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setByteOrder(QDataStream::BigEndian);
    out << id << len;
    block.append(dataByte);
    _sock.write(block);
}

void TcpMgr::slot_tcp_connect(ServerInfo si)
{
    qDebug() << "Ready to connect Server";
    _host = si.host;
    _port = static_cast<uint16_t>(si.port.toInt());
    _sock.connectToHost(si.host, _port);
}

TcpThread::TcpThread()
{
    _tcp_thread = new QThread();
    TcpMgr::GetInstance()->moveToThread(_tcp_thread);
    connect(_tcp_thread, &QThread::finished, _tcp_thread, &QObject::deleteLater);

    _tcp_thread->start();
}

TcpThread::~TcpThread()
{
    _tcp_thread->quit();
}
