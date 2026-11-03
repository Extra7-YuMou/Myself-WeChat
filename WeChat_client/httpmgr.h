#ifndef HTTPMGR_H
#define HTTPMGR_H
#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include "Singleton.h"
#include "Global.h"
#include <QString>
#include <QByteArray>
#include <QUrl>
#include <QNetworkReply>

/******************************************************************************
 *
 * @file            httpmgr.h
 * @brief           http管理类
 * @description     用于处理有关http请求
 *
 * @author          YuHaoNan
 * @date            2026/04/13
 * @history
 *****************************************************************************/
class HttpMgr : public QObject, public Singleton<HttpMgr>, public std::enable_shared_from_this<HttpMgr>
{
    Q_OBJECT
    friend class Singleton<HttpMgr>;
public:
    ~HttpMgr();
    void PostHttpReq(QUrl url, QJsonObject jsonObj, ReqId id, Modules mode);

private:
    HttpMgr();
    QNetworkAccessManager _manager;

private slots:
    void slot_http_finish(ReqId id, QString res, ErrorCodes err, Modules mod);

signals:
    void sig_http_finish(ReqId id, QString res, ErrorCodes err, Modules mod);
    void sig_reg_mod_finish(ReqId id, QString res, ErrorCodes err);
    void sig_reset_mod_finish(ReqId id, QString res, ErrorCodes err);
    void sig_login_mod_finish(ReqId id, QString res, ErrorCodes err);
};

#endif // HTTPMGR_H
