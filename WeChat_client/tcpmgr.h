#ifndef TCPMGR_H
#define TCPMGR_H
#include <QObject>
#include "Singleton.h"
#include "Global.h"
#include <QTcpSocket>
#include <atomic>
#include "userdata.h"
#include <QThread>

class TcpThread : public QThread{
    Q_OBJECT
public:
    TcpThread();
    ~TcpThread();
private:
    QThread* _tcp_thread;
};

/******************************************************************************
 *
 * @file            tcpmgr.h
 * @brief           TcpMgr类
 * @description     用于处理tcp连接，聊天服务器（长连接）
 *
 * @author          YuHaoNan
 * @date            2026/04/17
 * @history
 *****************************************************************************/
class TcpMgr : public QObject, public Singleton<TcpMgr>,
               std::enable_shared_from_this<TcpMgr>{
    Q_OBJECT
    friend class Singleton<TcpMgr>;
    using handler = std::function<void(TCP_ID_LEN, TCP_BODY_LEN, QByteArray)>;
private:
    TcpMgr();
    void initHandlers();
    void handleMsg(int, int, QByteArray);

    QTcpSocket _sock;
    QString _host;
    uint16_t _port;
    bool _b_recv_head;
    QByteArray _buffer;
    TCP_ID_LEN _message_id;
    TCP_BODY_LEN _message_len;
    QMap<int, handler> _handlers;

public slots:
    void slot_send_data(ReqId id, QString data);
    void slot_tcp_connect(ServerInfo si);

signals:
    void sig_con_success();
    void sig_send_data(ReqId id, QString data);
    void sig_login_failed(int err);
    void switchChat(std::vector<std::shared_ptr<ApplyInfo>>, QMap<int, std::shared_ptr<UserInfo>>, qint64, QMap<qint64, int>);
    void sig_search_user_failed(int err);
    void sig_search_user_success(std::shared_ptr<SearchInfo>);
    void sig_apply_friend_failed(int err);
    void sig_apply_friend_success(std::shared_ptr<AddFriendApply>);
    void sig_auth_friend_failed(int err);
    void sig_auth_friend_success(std::shared_ptr<ChatThreadData>);
    void sig_notify_friend_success(std::shared_ptr<ChatThreadData>);
    void sig_text_chat_msg(QVector<std::shared_ptr<ChatDataBase>>, int recv_uid);
    void sig_notify_text_chat_msg(QVector<std::shared_ptr<ChatDataBase>>, int recv_uid);
    void sig_notify_off_line(int);
    void sig_load_chat_thread();
    void sig_close_tip_wid();
    void sig_load_chat_message(QVector<std::shared_ptr<ChatDataBase>>);
    void sig_upload_img_chat_message(std::shared_ptr<ImgChatData>);
    void sig_download_file(qint64, qint64, QString, QString, int);
    void sig_notify_download_file(int send_uid, qint64 thread_id, qint64 msg_id, QString name, QString md5);
    void sig_upload_video_chat_msg(std::shared_ptr<VideoChatData>);
    void sig_download_video_chat_msg(int send_uid, qint64 thread_id, qint64 msg_id, QString name, QString coverName, int duration);
};

#endif // TCPMGR_H
