#include "filetcpmgr.h"
#include "usermgr.h"
#include <QDir>

FileTcpMgr::FileTcpMgr(QObject *parent)
    : QObject{parent}, _b_recv_head(false), _message_id(0), _message_len(0),
    _bytes_send(0), _host(""), _port(0)
{
    InitHandlers();

    connect(&_sock, &QTcpSocket::connected, [this](){
        qDebug() << "Connected to resources server";
    });

    connect(&_sock, &QTcpSocket::disconnected, [this](){
        qDebug() << "Resources server has not connected";
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
                qDebug() << "message id: " << _message_id << ", message len: " << _message_len;
                _buffer = _buffer.mid(TCP_HEAD_LEN);
            }

            if(_buffer.size() < _message_len){
                _b_recv_head = true;
                return;
            }

            _b_recv_head = false;
            QByteArray message_body = _buffer.mid(0, _message_len);
            _buffer = _buffer.mid(_message_len);

            HandleMsg(_message_id, _message_len, message_body);
        }
    });

    connect(&_sock, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred), [this](){
        qDebug() << "Error occured! Error: " << _sock.errorString();
    });

    connect(this, &FileTcpMgr::sig_send_msg, this, &FileTcpMgr::slot_send_msg);

    connect(&_sock, &QTcpSocket::bytesWritten, [this](qint64 bytes){
        _bytes_send += bytes;

        if(_bytes_send < _current_block.size()){
            auto mid_block = _current_block.mid(_bytes_send);
            _sock.write(mid_block);
            return;
        }

        if(_send_que.isEmpty()){
            _bytes_send = 0;
            return;
        }

        _current_block = _send_que.dequeue();
        _bytes_send = 0;
        _sock.write(_current_block);
    });
}

void FileTcpMgr::SendMsg(ReqId id, QByteArray data)
{
    emit sig_send_msg(id, data);
}

QString CheckThreadDir(std::shared_ptr<FileInfo> fileInfo){
    QString standardDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(standardDir);
    QRegularExpression regexp("thread_id_\\d+");
    QRegularExpressionMatch match = regexp.match(fileInfo->_name);
    QString dir_name;
    if(match.hasMatch()){
        dir_name = match.captured(0);
    }else{
        qDebug() << "不是聊天会话文件";
        return QString();
    }

    qDebug() << "dir name: " << dir_name;
    if(!dir.exists(dir_name)){
        if(!dir.mkpath(dir_name)){
            qDebug() << "创建" << dir_name << "文件夹失败";
            return QString();
        }else{
            qDebug() << "创建" << dir_name << "文件夹成功";
        }
    }
    return standardDir + dir_name;
}

void FileTcpMgr::InitHandlers()
{
    _handlers[ID_UPLOAD_HEAD_ICON_RSP] = [this](TCP_ID_LEN id, TCP_BODY_LEN len, QByteArray msg_data){
        QJsonDocument doc = QJsonDocument::fromJson(msg_data);
        if(doc.isNull()){
            qDebug() << "Parse upload head icon rsp failed";
            return;
        }

        QJsonObject json = doc.object();
        if(!json.contains("error")){
            qDebug() << "parse upload head icon rsp failed";
            return;
        }

        auto err = json["error"].toInt();
        if(err != ErrorCodes::SUCCESS){
            qDebug() << "Parse upload head icon occured error, error code: " << err;
            return;
        }

        if(json.contains("finish")){
            qDebug() << "Head icon upload finished";
            return;
        }

        auto md5 = json["md5"].toString();
        auto name = json["name"].toString();
        auto seq = json["seq"].toInt();
        auto trans_size = json["trans_size"].toInt();
        auto total_size = json["total_size"].toInteger();
        auto finished = json["finished"].toBool();
        seq++;
        qDebug() << "seq: " << seq << ", trans_size: " << trans_size << ", total_size: " << total_size
                 << "finished: " << finished;

        emit sig_trans_size(trans_size);
        if(finished){
            return;
        }

        auto fileInfo = UserMgr::GetInstance()->GetFileInfoByMd5(md5);
        if(!fileInfo){
            qDebug() << "Not find the file info";
            return;
        }

        QFile file(fileInfo->filePath());
        if(!file.open(QIODevice::ReadOnly)){
            qDebug() << "Open the file failed";
            return;
        }

        file.seek(trans_size);
        QByteArray buffer = file.read(MAX_FILE_READ_LEN);
        QString base64Data = buffer.toBase64();

        QJsonObject root;
        root["md5"] = md5;
        root["name"] = name;
        root["seq"] = seq;
        root["trans_size"] = buffer.size() + (seq - 1) * MAX_FILE_READ_LEN;
        root["total_size"] = total_size;
        root["data"] = base64Data;
        root["uid"] = UserMgr::GetInstance()->getUid();

        if(trans_size >= total_size){
            root["finished"] = true;
        }else{
            root["finished"] = false;
        }

        QByteArray send_msg = QJsonDocument(root).toJson(QJsonDocument::Compact);
        SendMsg(ReqId::ID_UPLOAD_HEAD_ICON_REQ, send_msg);

        file.close();
    };

    _handlers[ID_DOWNLOAD_FILE_RSP] = [this](TCP_ID_LEN id, TCP_BODY_LEN len, QByteArray msg_data){
        QJsonDocument doc = QJsonDocument::fromJson(msg_data);
        if(doc.isNull()){
            qDebug() << "Parse upload file rsp failed";
            return;
        }

        QJsonObject json = doc.object();
        if(!json.contains("error")){
            qDebug() << "Upload file rsp message not have error";
            return;
        }

        int err = json["error"].toInt();
        if(err != ErrorCodes::SUCCESS){
            qDebug() << "Receive upload file rsp has occured error, error: " << err;
            return;
        }

        auto seq = json["seq"].toInt();
        auto name = json["name"].toString();
        auto trans_size = json["trans_size"].toInt();
        auto total_size = json["total_size"].toInteger();
        auto base64Data = json["data"].toString();
        auto finished = json["finished"].toBool();
        qDebug() << "name: " << name << ", seq: " << seq << ", finished: " << finished;
        seq++;

        auto fileInfo = UserMgr::GetInstance()->GetDownLoadFile(name);
        if(fileInfo == nullptr){
            qDebug() << name << " not exists";
            return;
        }

        fileInfo->_trans_size = trans_size;
        fileInfo->_seq = seq;
        fileInfo->_total_size = total_size;

        QIODevice::OpenMode mode;
        if(seq == 1){
            mode = QIODevice::WriteOnly;
        }else{
            mode = QIODevice::WriteOnly | QIODevice::Append;
        }

        QString targetPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
                             "/avatars/" + UserMgr::GetInstance()->getIcon();
        QFile file(targetPath);
        if(!file.open(mode)){
            qDebug() << "Open file: " << targetPath << " failed, error"
                     << file.errorString();
            return;
        }

        // 文件数据解码
        QByteArray file_data = QByteArray::fromBase64(base64Data.toUtf8());
        qint64 writeBytes = file.write(file_data);
        if(writeBytes < base64Data.size()){
            qDebug() << "Failed to write all data. Written:" << writeBytes
                     << "Expected:" << base64Data.size();
        }

        file.close();

        if(finished){
            UserMgr::GetInstance()->RmvDownLoadFile(name);
            // 发送信号通知更新资源
            emit sig_reset_label_icon(name);
        }else{
            // 组织新一轮请求
            QJsonObject root;
            root["name"] = name;
            root["seq"] = seq;
            root["trans_size"] = trans_size;
            root["total_size"] = total_size;
            QByteArray data = QJsonDocument(root).toJson(QJsonDocument::Compact);

            SendMsg(ID_DOWNLOAD_FILE_REQ, data);
        }
    };

    _handlers[ID_UPLOAD_IMG_CHAT_MSG_RSP] = [this](TCP_ID_LEN id, TCP_BODY_LEN len, QByteArray msg_data){
        QJsonDocument doc = QJsonDocument::fromJson(msg_data);
        if(doc.isNull()){
            qDebug() << "Parse upload file rsp failed";
            return;
        }

        QJsonObject json = doc.object();
        if(!json.contains("error")){
            qDebug() << "Upload file rsp message not have error";
            return;
        }

        int err = json["error"].toInt();
        if(err != ErrorCodes::SUCCESS){
            qDebug() << "Receive upload file rsp has occured error, error: " << err;
            return;
        }

        auto md5 = json["md5"].toString();
        auto name = json["name"].toString();
        auto seq = json["seq"].toInt();
        auto trans_size = json["trans_size"].toInt();
        auto total_size = json["total_size"].toInteger();
        auto thread_id = json["thread_id"].toInteger();
        auto finished = json["finished"].toBool();
        auto msg_id = json["message_id"].toInteger();
        auto uid = json["uid"].toInt();
        auto recv_uid = json["recv_uid"].toInt();
        qDebug() << "name: " << name << ", seq: " << seq << ", finished: " << finished;
        seq++;

        if(finished){
            return;
        }

        auto fileInfo = UserMgr::GetInstance()->GetFileInfoByMd5(md5);
        if(!fileInfo){
            qDebug() << "The file info not exists, file name: " << name;
            return;
        }

        QFile file(fileInfo->filePath());
        if(!file.open(QIODevice::ReadOnly)){
            qDebug() << "Open file failed, file name: " << name;
            return;
        }

        file.seek(trans_size);

        QByteArray originalData = file.read(MAX_FILE_READ_LEN);
        QString base64Data = originalData.toBase64();

        QJsonObject root;
        root["md5"] = md5;
        root["name"] = name;
        root["seq"] = seq;
        root["trans_size"] = originalData.size() + (seq - 1) * MAX_FILE_READ_LEN;
        root["total_size"] = total_size;
        root["thread_id"] = thread_id;
        root["data"] = base64Data;
        root["message_id"] = msg_id;
        root["uid"] = uid;
        root["recv_uid"] = recv_uid;

        if(originalData.size() + (seq - 1) * MAX_FILE_READ_LEN >= total_size){
            root["finished"] = true;
        }else{
            root["finished"] = false;
        }

        QByteArray send_data = QJsonDocument(root).toJson(QJsonDocument::Compact);
        SendMsg(ReqId::ID_UPLOAD_IMG_CHAT_MSG_REQ, send_data);
        file.close();
    };

    _handlers[ID_DOWNLOAD_IMG_CHAT_MSG_RSP] = [this](TCP_ID_LEN id, TCP_BODY_LEN len, QByteArray msg_data){
        QJsonDocument doc = QJsonDocument::fromJson(msg_data);
        if(doc.isNull()){
            qDebug() << "Parse download chat img file rsp failed";
            return;
        }

        QJsonObject json = doc.object();
        if(!json.contains("error")){
            qDebug() << "Download chat img file rsp message not have error";
            return;
        }

        int err = json["error"].toInt();
        if(err != ErrorCodes::SUCCESS){
            qDebug() << "Receive download chat img file rsp has occured error, error: " << err;
            return;
        }

        auto msg_id = json["message_id"].toInteger();
        auto name = json["name"].toString();
        auto seq = json["seq"].toInt();
        auto trans_size = json["trans_size"].toInt();
        auto total_size = json["total_size"].toInteger();
        auto dir_name = json["dir_name"].toString();
        auto thread_id = json["thread_id"].toInteger();
        auto send_uid = json["send_uid"].toInt();
        auto originalData = json["data"].toString();
        auto finished = json["finished"].toBool();
        qDebug() << "Download file: name: " << name << ", seq: " << seq << ", dir_name: " << dir_name << ", finished: " << finished;
        seq++;

        auto fileInfo = UserMgr::GetInstance()->GetDownLoadFile(name);
        if(fileInfo == nullptr){
            qDebug() << name << " not exists";
            return;
        }

        fileInfo->_seq = seq;
        fileInfo->_trans_size = trans_size;
        fileInfo->_total_size = total_size;

        QByteArray fileData = QByteArray::fromBase64(originalData.toUtf8());

        QString targetDir = CheckThreadDir(fileInfo);

        QIODevice::OpenMode mode;
        if(seq == 1){
            mode = QIODevice::WriteOnly;
        }else{
            mode = QIODevice::WriteOnly | QIODevice::Append;
        }

        QFile file(fileInfo->_name);
        if(!file.open(mode)){
            qDebug() << "Open the file: " << fileInfo->_name << " failed, error: "
                     << file.errorString();
            return;
        }

        qint64 writeBytes = file.write(fileData);
        if(writeBytes < originalData.size()){
            qDebug() << "Write all data to file, write: " << writeBytes
                     << ", excepted: " << originalData.size();
        }

        file.close();

        if(finished){
            qDebug() << "文件下载完成";
            QString total_path = GetLocalFilePrefix(thread_id) + name;
            qDebug() << "文件地址: " << total_path;
            auto img_data = std::make_shared<ImgChatData>(msg_id, thread_id, ChatMsgType::Img, name, send_uid, total_path);

            UserMgr::GetInstance()->RmvDownLoadFile(name);
            emit sig_download_img_msg_chat_finished(thread_id, img_data);
        }else{
            QJsonObject root;
            root["name"] = name;
            root["seq"] = seq;
            root["trans_size"] = writeBytes + (seq - 1) * MAX_FILE_READ_LEN;
            root["total_size"] = total_size;
            root["dir_name"] = dir_name;
            QByteArray send_data = QJsonDocument(root).toJson(QJsonDocument::Compact);
            FileTcpMgr::GetInstance()->SendMsg(ReqId::ID_DOWNLOAD_IMG_CHAT_MSG_REQ, send_data);
        }
    };

    _handlers[ID_UPLOAD_VIDEO_CHAT_MSG_RSP] = [this](TCP_ID_LEN id, TCP_BODY_LEN len, QByteArray msg_data){
        QJsonDocument doc = QJsonDocument::fromJson(msg_data);
        if(doc.isNull()){
            qDebug() << "Parse upload file rsp failed";
            return;
        }

        QJsonObject json = doc.object();
        if(!json.contains("error")){
            qDebug() << "Upload file rsp message not have error";
            return;
        }

        int err = json["error"].toInt();
        if(err != ErrorCodes::SUCCESS){
            qDebug() << "Receive upload file rsp has occured error, error: " << err;
            return;
        }

        auto md5 = json["md5"].toString();
        auto name = json["name"].toString();
        auto seq = json["seq"].toInt();
        auto trans_size = json["trans_size"].toInt();
        auto total_size = json["total_size"].toInteger();
        auto thread_id = json["thread_id"].toInteger();
        auto finished = json["finished"].toBool();
        auto msg_id = json["message_id"].toInteger();
        auto uid = json["uid"].toInt();
        auto recv_uid = json["recv_uid"].toInt();
        qDebug() << "name: " << name << ", seq: " << seq << ", finished: " << finished;
        seq++;

        if(finished){
            return;
        }

        auto fileInfo = UserMgr::GetInstance()->GetFileInfoByMd5(md5);
        if(!fileInfo){
            qDebug() << "The file info not exists, file name: " << name;
            return;
        }

        QFile file(fileInfo->filePath());
        if(!file.open(QIODevice::ReadOnly)){
            qDebug() << "Open file failed, file name: " << name;
            return;
        }

        file.seek(trans_size);

        QByteArray originalData = file.read(MAX_FILE_READ_LEN);
        QString base64Data = originalData.toBase64();

        QJsonObject root;
        root["md5"] = md5;
        root["name"] = name;
        root["seq"] = seq;
        root["trans_size"] = originalData.size() + (seq - 1) * MAX_FILE_READ_LEN;
        root["total_size"] = total_size;
        root["thread_id"] = thread_id;
        root["data"] = base64Data;
        root["message_id"] = msg_id;
        root["uid"] = uid;
        root["recv_uid"] = recv_uid;

        if(originalData.size() + (seq - 1) * MAX_FILE_READ_LEN >= total_size){
            root["finished"] = true;
        }else{
            root["finished"] = false;
        }

        QByteArray send_data = QJsonDocument(root).toJson(QJsonDocument::Compact);
        SendMsg(ReqId::ID_UPLOAD_VIDEO_CHAT_MSG_REQ, send_data);
        file.close();
    };

    _handlers[ID_DOWNLOAD_VIDEO_COVER_CHAT_MSG_RSP] = [this](TCP_ID_LEN id, TCP_BODY_LEN len, QByteArray msg_data){
        QJsonDocument doc = QJsonDocument::fromJson(msg_data);
        if(doc.isNull()){
            qDebug() << "Parse download chat img file rsp failed";
            return;
        }

        QJsonObject json = doc.object();
        if(!json.contains("error")){
            qDebug() << "Download chat img file rsp message not have error";
            return;
        }

        int err = json["error"].toInt();
        if(err != ErrorCodes::SUCCESS){
            qDebug() << "Receive download chat img file rsp has occured error, error: " << err;
            return;
        }

        auto msg_id = json["message_id"].toInteger();
        auto name = json["name"].toString();
        auto seq = json["seq"].toInt();
        auto trans_size = json["trans_size"].toInt();
        auto total_size = json["total_size"].toInteger();
        auto dir_name = json["dir_name"].toString();
        auto thread_id = json["thread_id"].toInteger();
        auto send_uid = json["send_uid"].toInt();
        auto originalData = json["data"].toString();
        auto finished = json["finished"].toBool();
        qDebug() << "Download file: name: " << name << ", seq: " << seq << ", dir_name: " << dir_name << ", finished: " << finished;
        seq++;

        auto fileInfo = UserMgr::GetInstance()->GetDownLoadFile(name);
        if(fileInfo == nullptr){
            qDebug() << name << " not exists";
            return;
        }

        fileInfo->_seq = seq;
        fileInfo->_trans_size = trans_size;
        fileInfo->_total_size = total_size;

        QByteArray fileData = QByteArray::fromBase64(originalData.toUtf8());

        QString targetDir = CheckThreadDir(fileInfo);

        QIODevice::OpenMode mode;
        if(seq == 1){
            mode = QIODevice::WriteOnly;
        }else{
            mode = QIODevice::WriteOnly | QIODevice::Append;
        }

        QFile file(fileInfo->_name);
        if(!file.open(mode)){
            qDebug() << "Open the file: " << fileInfo->_name << " failed, error: "
                     << file.errorString();
            return;
        }

        qint64 writeBytes = file.write(fileData);
        if(writeBytes < originalData.size()){
            qDebug() << "Write all data to file, write: " << writeBytes
                     << ", excepted: " << originalData.size();
        }

        file.close();

        if(finished){
            qDebug() << "文件下载完成";
            QString total_path = GetDownLoadFilePrefix() + QDir::separator() + dir_name + QDir::separator() + name;
            qDebug() << "文件地址: " << total_path;
            UserMgr::GetInstance()->RmvDownLoadFile(name);
        }else{
            QJsonObject root;
            root["name"] = name;
            root["seq"] = seq;
            root["trans_size"] = writeBytes + (seq - 1) * MAX_FILE_READ_LEN;
            root["total_size"] = total_size;
            root["dir_name"] = dir_name;
            QByteArray send_data = QJsonDocument(root).toJson(QJsonDocument::Compact);
            FileTcpMgr::GetInstance()->SendMsg(ReqId::ID_DOWNLOAD_VIDEO_COVER_CHAT_MSG_REQ, send_data);
        }
    };

    _handlers[ID_DOWNLOAD_VIDEO_BODY_CHAT_MSG_RSP] = [this](TCP_ID_LEN id, TCP_BODY_LEN len, QByteArray msg_data){
        QJsonDocument doc = QJsonDocument::fromJson(msg_data);
        if(doc.isNull()){
            qDebug() << "Parse download chat img file rsp failed";
            return;
        }

        QJsonObject json = doc.object();
        if(!json.contains("error")){
            qDebug() << "Download chat img file rsp message not have error";
            return;
        }

        int err = json["error"].toInt();
        if(err != ErrorCodes::SUCCESS){
            qDebug() << "Receive download chat img file rsp has occured error, error: " << err;
            return;
        }

        auto msg_id = json["message_id"].toInteger();
        auto name = json["name"].toString();
        auto seq = json["seq"].toInt();
        auto trans_size = json["trans_size"].toInt();
        auto total_size = json["total_size"].toInteger();
        auto dir_name = json["dir_name"].toString();
        auto thread_id = json["thread_id"].toInteger();
        auto send_uid = json["send_uid"].toInt();
        auto originalData = json["data"].toString();
        auto finished = json["finished"].toBool();
        qDebug() << "Download file: name: " << name << ", seq: " << seq << ", dir_name: " << dir_name << ", finished: " << finished;
        seq++;

        auto fileInfo = UserMgr::GetInstance()->GetDownLoadFile(name);
        if(fileInfo == nullptr){
            qDebug() << name << " not exists";
            return;
        }

        fileInfo->_seq = seq;
        fileInfo->_trans_size = trans_size;
        fileInfo->_total_size = total_size;

        QByteArray fileData = QByteArray::fromBase64(originalData.toUtf8());

        QString targetDir = CheckThreadDir(fileInfo);

        QIODevice::OpenMode mode;
        if(seq == 1){
            mode = QIODevice::WriteOnly;
        }else{
            mode = QIODevice::WriteOnly | QIODevice::Append;
        }

        QFile file(fileInfo->_name);
        if(!file.open(mode)){
            qDebug() << "Open the file: " << fileInfo->_name << " failed, error: "
                     << file.errorString();
            return;
        }

        qint64 writeBytes = file.write(fileData);
        if(writeBytes < originalData.size()){
            qDebug() << "Write all data to file, write: " << writeBytes
                     << ", excepted: " << originalData.size();
        }

        file.close();

        if(finished){
            qDebug() << "文件下载完成";
            QString total_path = GetLocalFilePrefix(thread_id) + name;
            QString coverName = name;
            coverName.replace(QRegularExpression("\\.[^.]+$"), "_cover.jpg");
            QString coverPath = GetDownLoadFilePrefix() + QDir::separator() + "video_covers" + QDir::separator() + coverName;
            qDebug() << "文件地址: " << total_path;
            auto videoData = std::make_shared<VideoChatData>(msg_id, thread_id, ChatMsgType::Video, name, send_uid, total_path, coverPath, 0);
            UserMgr::GetInstance()->RmvDownLoadFile(name);
            emit sig_download_video_msg_chat_finished(thread_id, videoData);
        }else{
            QJsonObject root;
            root["name"] = name;
            root["seq"] = seq;
            root["trans_size"] = writeBytes + (seq - 1) * MAX_FILE_READ_LEN;
            root["total_size"] = total_size;
            root["dir_name"] = dir_name;
            QByteArray send_data = QJsonDocument(root).toJson(QJsonDocument::Compact);
            FileTcpMgr::GetInstance()->SendMsg(ReqId::ID_DOWNLOAD_VIDEO_COVER_CHAT_MSG_REQ, send_data);
        }
    };
}

void FileTcpMgr::HandleMsg(TCP_ID_LEN id, TCP_BODY_LEN len, QByteArray data)
{
    auto iter_find = _handlers.find(id);
    if(iter_find == _handlers.end()){
        qDebug() << "Not found match handler";
        return;
    }
    iter_find.value()(id, len, data);
}

void FileTcpMgr::slot_send_msg(ReqId id, QByteArray data)
{
    quint16 _id = id;
    TCP_BODY_LEN len = static_cast<TCP_BODY_LEN>(data.size());

    QByteArray push_data;
    QDataStream ds(&push_data, QDataStream::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);

    ds << _id << len;
    push_data.append(data);

    _send_que.enqueue(push_data);
    if(_send_que.size() > 1){
        return;
    }

    _current_block = _send_que.dequeue();
    _bytes_send = 0;
    _sock.write(_current_block);
}

void FileTcpMgr::slot_connect_file_tcp(ServerInfo si)
{
    _host  = si.host;
    _port = si.port.toUShort();
    _sock.connectToHost(_host, _port);
}

FileTcpThread::FileTcpThread()
{
    _file_thread = new QThread();
    FileTcpMgr::GetInstance()->moveToThread(_file_thread);
    connect(_file_thread, &QThread::finished, _file_thread, &QThread::deleteLater);

    _file_thread->start();
}

FileTcpThread::~FileTcpThread()
{
    _file_thread->quit();
}
