#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include "Global.h"

namespace Ui {
class LoginDialog;
}


/******************************************************************************
 *
 * @file            logindialog.h
 * @brief           登录窗口
 * @description     可以在这里实现登录功能
 *
 * @author          YuHaoNan
 * @date            2026/04/13
 * @history
 *****************************************************************************/
class LoginDialog : public QDialog
{
    Q_OBJECT
    using Handler = std::function<void(const QJsonObject& json)>;
public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

private slots:
    void slot_login_mod_finish(ReqId id, QString res, ErrorCodes err);
    void slot_tcp_connect_finish();
    void on_login_btn_clicked();
    void slot_login_failed(int err);

signals:
    void switchRegister();
    void switchReset();
    void sig_connect_tcp(ServerInfo);
    void sig_connect_file_tcp(ServerInfo);

private:
    void initHandlers();
    void showTip(QString tip, bool result);

    Ui::LoginDialog *ui;
    QMap<ReqId,  Handler> _handlers;
    int _uid;
    QString _token;
};

#endif // LOGINDIALOG_H
