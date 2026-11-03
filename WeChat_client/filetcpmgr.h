#ifndef FILETCPMGR_H
#define FILETCPMGR_H

#include <QObject>
#include <QTcpSocket>
#include "Singleton.h"
#include <QThread>
#include <QQueue>
#include "global.h"
#include <QMap>
#include "userdata.h"

class FileTcpThread : public QThread{
    Q_OBJECT
public:
    FileTcpThread();
    ~FileTcpThread();
private:
    QThread* _file_thread;
};

class FileTcpMgr : public QObject, public Singleton<FileTcpMgr>
{
    Q_OBJECT
    friend class Singleton<FileTcpMgr>;
public:
    ~FileTcpMgr() = default;
    void SendMsg(ReqId id, QByteArray data);

private:
    explicit FileTcpMgr(QObject *parent = nullptr);
    void InitHandlers();
    void HandleMsg(TCP_ID_LEN id, TCP_BODY_LEN len, QByteArray data);

    QTcpSocket _sock;
    QString _host;
    uint16_t _port;
    bool _b_recv_head;
    qint64 _bytes_send;
    QQueue<QByteArray> _send_que;
    QByteArray _current_block;
    qint16 _message_id;
    qint32 _message_len;
    QByteArray _buffer;
    QMap<qint16, std::function<void(TCP_ID_LEN, TCP_BODY_LEN, QByteArray)>> _handlers;

signals:
    void sig_send_msg(ReqId id, QByteArray data);
    void sig_trans_size(int trans_size);
    void sig_reset_label_icon(const QString&);
    void sig_upload_img_msg_chat_finished();
    void sig_download_img_msg_chat_finished(qint64, std::shared_ptr<ChatDataBase>);
    void sig_download_video_msg_chat_finished(qint64, std::shared_ptr<VideoChatData>);
public slots:
    void slot_connect_file_tcp(ServerInfo);
private slots:
    void slot_send_msg(ReqId id, QByteArray data);
};

#endif // FILETCPMGR_H
