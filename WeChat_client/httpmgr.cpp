#include "httpmgr.h"

HttpMgr::~HttpMgr()
{

}

HttpMgr::HttpMgr() {
    connect(this, &HttpMgr::sig_http_finish, this, &HttpMgr::slot_http_finish);
}

void HttpMgr::PostHttpReq(QUrl url, QJsonObject jsonObj, ReqId id, Modules mod)
{
    QByteArray data = QJsonDocument(jsonObj).toJson();
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setHeader(QNetworkRequest::ContentLengthHeader, QByteArray::number(data.length()));
    auto self = shared_from_this();
    QNetworkReply* reply = _manager.post(req, data);
    connect(reply, &QNetworkReply::finished, [self, reply, id, mod]{
        if(reply->error()){
            qDebug() << reply->errorString();
            emit self->sig_http_finish(id, "", ErrorCodes::ERR_NETWORK, mod);
            reply->deleteLater();
            return;
        }

        QString res = reply->readAll();
        emit self->sig_http_finish(id, res, ErrorCodes::SUCCESS, mod);
        reply->deleteLater();
        return;
    });
}

void HttpMgr::slot_http_finish(ReqId id, QString res, ErrorCodes err, Modules mod)
{
    if(mod == Modules::REGISTERMOD){
        emit sig_reg_mod_finish(id, res, err);
    }

    if(mod == Modules::RESETMOD){
        emit sig_reset_mod_finish(id, res, err);
    }

    if(mod == Modules::LOGINMOD){
        emit sig_login_mod_finish(id, res, err);
    }
}
