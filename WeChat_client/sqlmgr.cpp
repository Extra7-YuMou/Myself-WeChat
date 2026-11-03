#include "sqlmgr.h"
#include "usermgr.h"
#include "global.h"
#include <QDir>

SqlMgr::SqlMgr(QObject *parent)
    : QObject{parent}, _last_chat_thread(0), _cur_chat_thread(0)
{
    getThreadLocalConnection();
    ExistsUserTable();
    ExistsChatMessageTeble();
    ExistsChatThreadTable();
    ExistsPrivateChatTable();
}

QSqlDatabase& SqlMgr::getThreadLocalConnection()
{
    struct DbWrapper {
        QSqlDatabase db;
        QString connName;
        ~DbWrapper() {
            if (db.isOpen()) {
                db.close();
            }
            QSqlDatabase::removeDatabase(connName);
        }
    };
    thread_local DbWrapper wrapper = []() {
        DbWrapper w;
        w.connName = QString("sqlite_%1").arg(reinterpret_cast<quintptr>(QThread::currentThread()));
        w.db = QSqlDatabase::addDatabase("QSQLITE", w.connName);
        w.db.setDatabaseName("wechat.db");
        if (!w.db.open()) {
            qWarning() << "Failed to open db in thread";
        }
        return w;
    }();
    return wrapper.db;
}

bool SqlMgr::ExistsUserTable()
{
    QSqlQuery query(getThreadLocalConnection());
    QString checkStr = "SELECT name FROM sqlite_master WHERE type='table' AND name='user'";
    if (query.exec(checkStr) && query.next()) {
        qDebug() << "user 表已存在";
        return true;
    }

    // 表不存在，开始建表
    QString createTable = R"(
        CREATE TABLE user (
            uid INT NOT NULL PRIMARY KEY,
            name VARCHAR(255) NOT NULL,
            nick VARCHAR(255) NOT NULL,
            icon VARCHAR(255) NOT NULL,
            desc VARCHAR(255),
            sex TINYINT NOT NULL
        )
    )";
    if (!query.exec(createTable)) {
        qDebug() << "chat_message 建表失败:" << query.lastError().text();
        return false;
    }

    qDebug() << "chat_message 表创建成功";
    return true;
}

bool SqlMgr::ExistsChatMessageTeble()
{
    QSqlQuery query(getThreadLocalConnection());
    QString checkStr = "SELECT name FROM sqlite_master WHERE type='table' AND name='chat_message'";
    if (query.exec(checkStr) && query.next()) {
        qDebug() << "chat_message 表已存在";
        return true;
    }

    // 表不存在，开始建表
    QString createTable = R"(
        CREATE TABLE chat_message (
            message_id INTEGER PRIMARY KEY AUTOINCREMENT,
            thread_id  INTEGER NOT NULL,
            sender_id  INTEGER NOT NULL,
            recv_id    INTEGER NOT NULL,
            type       TINYINT NOT NULL,
            content    TEXT NOT NULL,
            created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
            updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
            status     INTEGER NOT NULL DEFAULT 0
        )
    )";
    if (!query.exec(createTable)) {
        qDebug() << "chat_message 建表失败:" << query.lastError().text();
        return false;
    }

    // 分开创建索引
    QString index1 = "CREATE INDEX idx_thread_created ON chat_message(thread_id, created_at)";
    QString index2 = "CREATE INDEX idx_thread_message ON chat_message(thread_id, message_id)";
    query.exec(index1);  // 忽略已存在错误
    query.exec(index2);

    qDebug() << "chat_message 表创建成功";
    return true;
}

bool SqlMgr::ExistsChatThreadTable()
{
    QSqlQuery query(getThreadLocalConnection());
    QString checkStr = "SELECT name FROM sqlite_master WHERE type='table' AND name='chat_thread'";
    if (query.exec(checkStr) && query.next()) {
        qDebug() << "chat_thread 表已存在";
        return true;
    }

    QString createTable = R"(
        CREATE TABLE chat_thread (
            id INTEGER PRIMARY KEY,
            type TEXT NOT NULL CHECK(type IN ('private', 'group')),
            created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
        )
    )";
    if (!query.exec(createTable)) {
        qDebug() << "chat_thread 建表失败:" << query.lastError().text();
        return false;
    }

    qDebug() << "chat_thread 表创建成功";
    return true;
}

bool SqlMgr::ExistsPrivateChatTable()
{
    QSqlQuery query(getThreadLocalConnection());
    QString checkStr = "SELECT name FROM sqlite_master WHERE type='table' AND name='private_chat'";
    if (query.exec(checkStr) && query.next()) {
        qDebug() << "private_chat 表已存在";
        return true;
    }

    QString createTable = R"(
        CREATE TABLE private_chat (
            thread_id INTEGER PRIMARY KEY,
            user1_id INTEGER NOT NULL,
            user2_id INTEGER NOT NULL,
            created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
            UNIQUE(user1_id, user2_id)
        )
    )";
    if (!query.exec(createTable)) {
        qDebug() << "private_chat 建表失败:" << query.lastError().text();
        return false;
    }

    QString index1 = "CREATE INDEX idx_private_user1_thread ON private_chat(user1_id, thread_id)";
    QString index2 = "CREATE INDEX idx_private_user2_thread ON private_chat(user2_id, thread_id)";
    query.exec(index1);
    query.exec(index2);

    qDebug() << "private_chat 表创建成功";
    qDebug() << QSqlDatabase::database().tables();
    return true;
}

qint64 SqlMgr::GetLastThreadId()
{
    QSqlQuery query(getThreadLocalConnection());
    QString queryStr = "SELECT `id` FROM `chat_thread` ORDER BY `id` DESC LIMIT 1";
    if(!query.exec(queryStr)){
        return 0;
    }

    if(!query.next()){
        return 0;
    }

    _last_chat_thread = query.value("id").toLongLong();
    qDebug() << "last thread id: " << _last_chat_thread;
    return _last_chat_thread;
}

QVector<std::shared_ptr<ChatThreadInfo> > SqlMgr::GetDiffChatThread()
{
    QSqlQuery query(getThreadLocalConnection());
    QString queryStr = "SELECT * FROM private_chat WHERE thread_id > ? ORDER BY thread_id";
    query.prepare(queryStr);
    query.addBindValue(_cur_chat_thread);
    qDebug() << "cur chat thread id: " << _cur_chat_thread;
    query.exec();

    QVector<std::shared_ptr<ChatThreadInfo>> tmp;
    while(query.next()){
        auto info = std::make_shared<ChatThreadInfo>(
            query.value("thread_id").toLongLong(),
            query.value("user1_id").toInt(),
            query.value("user2_id").toInt());
        _cur_chat_thread = query.value("thread_id").toLongLong();
        tmp.push_back(info);
    }

    return tmp;
}

QVector<std::shared_ptr<ChatThreadInfo> > SqlMgr::GetChatThread()
{
    QVector<std::shared_ptr<ChatThreadInfo>> tmp;

    QSqlQuery query(getThreadLocalConnection());
    QString selectStr = "SELECT COUNT(*) FROM private_chat";
    query.exec(selectStr);
    if(!query.next()){
        qDebug() << "没有数据";
        return tmp;
    }


    QString queryStr = "SELECT * FROM private_chat WHERE user1_id = ? OR user2_id = ?";
    query.prepare(queryStr);
    int uid = UserMgr::GetInstance()->getUid();
    query.addBindValue(uid);
    query.addBindValue(uid);
    if(!query.exec()){
        qDebug() << "执行查询操作失败: " << query.lastError().text();
        return tmp;
    }

    while(query.next()){
        qDebug() << "thread_id: " << query.value("thread_id").toLongLong();
        auto info = std::make_shared<ChatThreadInfo>(
            query.value("thread_id").toLongLong(),
            query.value("user1_id").toInt(),
            query.value("user2_id").toInt());
        tmp.append(info);
    }

    return tmp;
}

bool SqlMgr::InsertChatThread(std::shared_ptr<ChatThreadInfo> data)
{
    if(!data){
        return false;
    }

    return InsertPrivateChat(data);
}

// bool SqlMgr::InsertPrivateChat(std::shared_ptr<ChatThreadInfo> &info)
// {
//     QSqlQuery query;
//     QString queryStr = "SELECT thread_id FROM chat_thread WHERE id = ?";
//     query.prepare(queryStr);
//     query.addBindValue(info->_thread_id);
//     query.exec();
//     if(query.next()){
//         return true;
//     }

//     qDebug() << "thread id: " << info->_thread_id;
//     queryStr = "INSERT INTO `chat_thread`(id, `type`) values(?, 'private')";
//     query.prepare(queryStr);
//     query.addBindValue(info->_thread_id);
//     if(!query.exec()){
//         return false;
//     }

//     queryStr = "INSERT INTO `private_chat`(thread_id, `user1_id`, `user2_id`) values(?, ?, ?)";
//     query.prepare(queryStr);
//     query.addBindValue(info->_thread_id);
//     query.addBindValue(info->_user1_id);
//     query.addBindValue(info->_user2_id);

//     if(!query.exec()){
//         return false;
//     }

//     return true;
// }

bool SqlMgr::InsertPrivateChat(std::shared_ptr<ChatThreadInfo> &info)
{
    if (!info) return false;

    QSqlQuery query(getThreadLocalConnection());

    // 1. 先检查 chat_thread 表中是否已存在该 ID
    QString checkStr = "SELECT id FROM chat_thread WHERE id = ?";
    query.prepare(checkStr);
    query.addBindValue(info->_thread_id);

    if (!query.exec()) {
        qDebug() << "检查 thread_id 失败:" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        return true;
    }

    // 2. 插入 chat_thread 表
    QString insertThreadStr = "INSERT INTO chat_thread (id, type) VALUES (?, 'private')";
    query.prepare(insertThreadStr);
    query.addBindValue(info->_thread_id);

    if (!query.exec()) {
        qDebug() << "插入 chat_thread 失败:" << query.lastError().text();
        return false;
    }

    // 3. 插入 private_chat 表
    QString insertPrivateStr = "INSERT INTO private_chat (thread_id, user1_id, user2_id) VALUES (?, ?, ?)";
    query.prepare(insertPrivateStr);
    query.addBindValue(info->_thread_id);
    query.addBindValue(info->_user1_id);
    query.addBindValue(info->_user2_id);

    if (!query.exec()) {
        qDebug() << "插入 private_chat 失败:" << query.lastError().text();
        return false;
    }

    query.prepare("SELECT * FROM private_chat WHERE thread_id = ?");
    query.addBindValue(info->_thread_id);
    query.exec();
    if(query.next()){
        qDebug() << "thread_id: " << query.value("thread_id").toLongLong()
                 << "user1_id: " << query.value("user1_id").toInt()
                 << "user2_id: " << query.value("user2_id").toInt();
    }else{
        qDebug() << "插入info数据失败";
    }

    _last_chat_thread = info->_thread_id;
    return true;
}

bool SqlMgr::InsertUserInfo(std::shared_ptr<UserInfo> info)
{
    if(info == nullptr){
        return false;
    }

    QSqlQuery query(getThreadLocalConnection());
    QString queryStr = "SELECT * FROM \"user\" WHERE uid = ?";
    query.prepare(queryStr);
    query.addBindValue(info->_uid);
    query.exec();
    if(query.next()){
        qDebug() << "已有uid: " << info->_uid << "的用户数据";
        return true;
    }

    queryStr = "INSERT INTO \"user\"(uid, name, nick, icon, sex) values(?,?,?,?,?);";
    query.prepare(queryStr);
    query.addBindValue(info->_uid);
    query.addBindValue(info->_name);
    query.addBindValue(info->_nick);
    query.addBindValue(info->_icon);
    query.addBindValue(info->_sex);
    if(!query.exec()){
        qDebug() << "插入用户数据失败";
        return false;
    }else{
        qDebug() << "插入用户数据成功";
    }

    return true;
}

std::shared_ptr<UserInfo> SqlMgr::GetUserInfo(const int &uid)
{
    QSqlQuery query(getThreadLocalConnection());
    QString queryStr = "SELECT * FROM user WHERE uid = ?;";
    query.prepare(queryStr);
    query.exec();

    if(query.next()){
        auto info = std::make_shared<UserInfo>(
            query.value("uid").toInt(),
            query.value("name").toString(),
            query.value("nick").toString(),
            query.value("icon").toString(),
            query.value("sex").toInt(),
            query.value("desc").toString(),
            query.value("back").toString());
        return info;
    }
    qDebug() << "GetUserInfo没有数据";
    return nullptr;
}

bool SqlMgr::InsertChatMessage(std::shared_ptr<ChatDataBase> data, int recv_uid)
{
    if(data == nullptr){
        return false;
    }

    QSqlQuery query(getThreadLocalConnection());
    {
        QString queryStr = "SELECT * FROM chat_thread WHERE id = ?";
        query.prepare(queryStr);
        query.addBindValue(data->GetThreadId());
        query.exec();
        if(!query.next()){
            qDebug() << "本地没有id: " << data->GetThreadId() << "的聊天会话";
            auto threadInfo = std::make_shared<ChatThreadInfo>(
                data->GetThreadId(), data->GetSendUid(), recv_uid);
            if(!InsertPrivateChat(threadInfo)){
                return false;
            }
        }
    }
    {
        QString queryStr = "INSERT INTO chat_message(message_id, thread_id, sender_id, recv_id, type, content, status) "
                           "VALUES(?,?,?,?,?,?,0)";
        query.prepare(queryStr);
        qDebug() << "ti: " << data->GetThreadId() << ", mi: " << data->GetMsgId()
                 << ", send_uid: " << data->GetSendUid() << ", recvId: " << recv_uid
                 << ", content: " << data->GetContent();
        query.addBindValue(data->GetMsgId());
        query.addBindValue(data->GetThreadId());
        query.addBindValue(data->GetSendUid());
        query.addBindValue(recv_uid);
        query.addBindValue(data->GetMsgType());
        query.addBindValue(data->GetContent());
        if(!query.exec()){
            qDebug() << "插入聊天消息失败";
            return false;
        }
        qDebug() << "插入聊天消息: id: " << data->GetMsgId()
                 << ", thread_id: " << data->GetThreadId()
                 << ", sender_id: " << data->GetSendUid()
                 << ", recv_id: " << recv_uid
                 << ", type: " << data->GetMsgType()
                 << ", content: " << data->GetContent() << " 成功";
    }
    return true;
}

std::shared_ptr<ChatThreadData> SqlMgr::GetChatThreadData(qint64 thread_id)
{
    QSqlQuery query(getThreadLocalConnection());
    QString queryStr = "SELECT * FROM chat_message WHERE thread_id = ?";
    query.prepare(queryStr);
    query.addBindValue(thread_id);
    query.exec();

    QMap<qint64, std::shared_ptr<ChatDataBase>> chat_data;
    auto self_uid = UserMgr::GetInstance()->getUid();
    int other_uid = 0;
    while(query.next()){
        if(query.value("type").toInt() == ChatMsgType::Text){
            chat_data[query.value("message_id").toLongLong()] = std::make_shared<ChatDataBase>(
                query.value("message_id").toLongLong(),
                thread_id,
                ChatMsgType(query.value("type").toInt()),
                query.value("content").toString(),
                query.value("sender_id").toInt());
        }else if(query.value("type").toInt() == ChatMsgType::Img){
            chat_data[query.value("message_id").toLongLong()] = std::make_shared<ImgChatData>(
                query.value("message_id").toLongLong(),
                thread_id,
                ChatMsgType(query.value("type").toInt()),
                query.value("content").toString(),
                query.value("sender_id").toInt(),
                GetLocalFilePrefix(query.value("thread_id").toLongLong()) + query.value("content").toString());
        }else if(query.value("type").toInt() == ChatMsgType::Video){
            auto videoPath = GetLocalFilePrefix(query.value("thread_id").toLongLong()) + query.value("content").toString();
            auto coverPath = GetDownLoadFilePrefix() + QDir::separator() + "video_covers" + QDir::separator() +
                             query.value("content").toString().replace(QRegularExpression("\\.[^.]+$"), "_cover.jpg");
            chat_data[query.value("message_id").toLongLong()] = std::make_shared<VideoChatData>(
                query.value("message_id").toLongLong(),
                thread_id,
                ChatMsgType(query.value("type").toInt()),
                query.value("content").toString(),
                query.value("sender_id").toInt(),
                videoPath,
                coverPath,
                extractVideoDuration(videoPath));
        }
    }

    if(chat_data.isEmpty()){
        return std::make_shared<ChatThreadData>(thread_id, 0, other_uid);
    }
    auto thread_data = std::make_shared<ChatThreadData>(thread_id, (--chat_data.end()).value()->GetMsgId(), other_uid);
    thread_data->GetChatData() = std::move(chat_data);
    return thread_data;
}

bool SqlMgr::GetChatThreadOtherUid(int &other_id, const qint64& thread_id)
{
    {
        QSqlQuery query(getThreadLocalConnection());
        QString queryStr = "SELECT * FROM private_chat WHERE thread_id = ?";
        query.prepare(queryStr);
        query.addBindValue(thread_id);
        if(!query.exec()){
            return false;
        }
        if(query.next()){
            auto self_uid = UserMgr::GetInstance()->getUid();
            if(query.value("user1_id").toInt() == self_uid){
                other_id = query.value("user2_id").toInt();
            }else{
                other_id = query.value("user1_id").toInt();
            }
        }
        if(other_id == 0){
            return false;
        }
        return true;
    }
}

QMap<qint64, std::shared_ptr<ChatDataBase> > SqlMgr::GetChatMessage(const qint64 &thread_id)
{
    {
        QSqlQuery query(getThreadLocalConnection());
        QString queryStr = "SELECT * FROM chat_message WHERE thread_id = ? ORDER BY message_id";
        query.prepare(queryStr);
        query.addBindValue(thread_id);
        query.exec();
        auto self_uid = UserMgr::GetInstance()->getUid();
        QMap<qint64, std::shared_ptr<ChatDataBase>> _data_list;
        while(query.next()){
            auto chat_data = std::make_shared<ChatDataBase>(
                query.value("message_id").toLongLong(),
                query.value("thread_id").toLongLong(),
                ChatMsgType(query.value("type").toInt()),
                query.value("content").toString(),
                query.value("sender_id").toInt());
            _data_list[query.value("message_id").toLongLong()] = chat_data;
        }
        qDebug() << "chat message list: thread_id: " << query.value("thread_id").toLongLong()
                 << ", size: " << _data_list.size();
        return _data_list;
    }
}

