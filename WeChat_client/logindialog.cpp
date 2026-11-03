#include "logindialog.h"
#include "ui_logindialog.h"
#include "global.h"
#include "HttpMgr.h"
#include "TcpMgr.h"
#include "usermgr.h"
#include "filetcpmgr.h"

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog), _uid(0), _token("")
{
    ui->setupUi(this);
    ui->pass_edit->setEchoMode(QLineEdit::Password);
    ui->forget_label->setState("unvisible", "unvisible_hover", "",
                               "visible", "visible_hover", "");
    ui->forget_label->setCursor(Qt::PointingHandCursor);
    connect(ui->forget_label, &ClickLabel::clicked, [this](){
        emit switchReset();
    });

    ui->pass_visible->setState("unvisible", "unvisible_hover", "",
                               "visible", "visible_hover", "");
    ui->pass_visible->setCursor(Qt::PointingHandCursor);
    connect(ui->pass_visible, &ClickLabel::clicked, [this](){
        auto state = ui->pass_visible->GetState();
        if(state == ClickState::normal){
            ui->pass_edit->setEchoMode(QLineEdit::Password);
        }else{
            ui->pass_edit->setEchoMode(QLineEdit::Normal);
        }
    });

    connect(ui->reg_btn, &QPushButton::clicked, this, &LoginDialog::switchRegister);
    connect(HttpMgr::GetInstance().get(), &HttpMgr::sig_login_mod_finish, this, &LoginDialog::slot_login_mod_finish);
    connect(this, &LoginDialog::sig_connect_tcp, TcpMgr::GetInstance().get(), &TcpMgr::slot_tcp_connect);
    connect(this, &LoginDialog::sig_connect_file_tcp, FileTcpMgr::GetInstance().get(), &FileTcpMgr::slot_connect_file_tcp);
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_con_success, this, &LoginDialog::slot_tcp_connect_finish);
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_login_failed, this, &LoginDialog::slot_login_failed);

    initHandlers();
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::slot_login_mod_finish(ReqId id, QString res, ErrorCodes err)
{
    if(err != ErrorCodes::SUCCESS){
        showTip(tr("网络请求错误"), false);
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(res.toUtf8());
    if(doc.isNull()){
        showTip(tr("Json解析错误"), false);
        return;
    }

    if(!doc.isObject()){
        showTip(tr("Json解析错误"), false);
        return;
    }

    _handlers[id](doc.object());
}

void LoginDialog::slot_tcp_connect_finish()
{
    showTip(tr("连接服务器成功，请稍后..."), true);
    // 发送请求给ChatServer
    QJsonObject json;
    json["uid"] = _uid;
    json["token"] = _token;

    QJsonDocument doc(json);
    QString data = doc.toJson(QJsonDocument::Indented);
    emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_CHAT_LOGIN, data);
}

void LoginDialog::initHandlers()
{
    _handlers[ReqId::ID_USER_LOGIN] = [this](const QJsonObject& json){
        auto error = json["error"].toInt();
        if(error != ErrorCodes::SUCCESS){
            showTip(tr("参数错误"), false);
            return;
        }
        auto user = json["user"].toString();
        ServerInfo si;
        si.uid = json["uid"].toInt();
        si.host = json["host"].toString();
        si.port = json["port"].toString();
        si.token = json["token"].toString();

        ServerInfo res_si;
        res_si.uid = json["uid"].toInt();
        res_si.host = json["res_host"].toString();
        res_si.port = json["res_port"].toString();
        _uid = si.uid;
        _token = si.token;
        qDebug()<< "user is " << user << " uid is " << si.uid <<" host is "
                 << si.host << " Port is " << si.port << " Token is " << si.token;
        UserMgr::GetInstance()->setToken(_token);
        emit sig_connect_tcp(si);
        emit sig_connect_file_tcp(res_si);
    };
}

void LoginDialog::showTip(QString tip, bool result)
{
    ui->tip_label->setText(tip);
    if(result){
        ui->tip_label->setProperty("state", "normal");
    }else{
        ui->tip_label->setProperty("state", "err");
    }
    repolish(ui->tip_label);
}

void LoginDialog::on_login_btn_clicked()
{
    if(ui->user_edit->text() == ""){
        return;
    }

    if(ui->pass_edit->text() == ""){
        return;
    }

    QString user = ui->user_edit->text();
    QString pwd = ui->pass_edit->text();

    QJsonObject json;
    json["user"] = user;
    json["pwd"] = xorString(pwd);
    HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix + "/user_login"),
                                        json, ReqId::ID_USER_LOGIN, Modules::LOGINMOD);
}

void LoginDialog::slot_login_failed(int err){
    QString err_tip = QString("登陆失败，错误码:") + QString::number(err);
    showTip(err_tip, false);
}

