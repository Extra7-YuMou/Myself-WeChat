#ifndef SQLMGR_H
#define SQLMGR_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "Singleton.h"
#include <QMessageBox>
#include "userdata.h"
#include <QVector>
#include <QThread>

class SqlMgr : public QObject, public Singleton<SqlMgr>
{
    Q_OBJECT
    friend class Singleton<SqlMgr>;
public:
    static QSqlDatabase& getThreadLocalConnection();
    bool ExistsUserTable();
    bool ExistsChatMessageTeble();
    bool ExistsChatThreadTable();
    bool ExistsPrivateChatTable();
    qint64 GetLastThreadId();
    QVector<std::shared_ptr<ChatThreadInfo>> GetDiffChatThread();
    QVector<std::shared_ptr<ChatThreadInfo>> GetChatThread();
    bool InsertChatThread(std::shared_ptr<ChatThreadInfo>);
    bool InsertPrivateChat(std::shared_ptr<ChatThreadInfo>& info);
    bool InsertUserInfo(std::shared_ptr<UserInfo>);
    std::shared_ptr<UserInfo> GetUserInfo(const int& uid);
    bool InsertChatMessage(std::shared_ptr<ChatDataBase>, int);
    std::shared_ptr<ChatThreadData> GetChatThreadData(qint64 thread_id);
    bool GetChatThreadOtherUid(int& other_id, const qint64& thread_id);
    QMap<qint64, std::shared_ptr<ChatDataBase>> GetChatMessage(const qint64& thread_id);
private:
    explicit SqlMgr(QObject *parent = nullptr);
    qint64 _last_chat_thread;
    qint64 _cur_chat_thread;
signals:
};

#endif // SQLMGR_H
