#include "resetdialog.h"
#include "ui_resetdialog.h"
#include <QRegularExpression>
#include <httpmgr.h>

ResetDialog::ResetDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ResetDialog)
{
    ui->setupUi(this);
    ui->tip_label->setProperty("state", "normal");
    repolish(ui->tip_label);

    connect(ui->user_edit, &QLineEdit::editingFinished, [this](){
        checkUser();
    });

    connect(ui->email_edit, &QLineEdit::editingFinished, [this](){
        checkEmail();
    });

    connect(ui->verify_edit, &QLineEdit::editingFinished, [this](){
        checkVerify();
    });

    connect(ui->newPwd_edit, &QLineEdit::editingFinished, [this](){
        checkNewPwd();
    });

    connect(HttpMgr::GetInstance().get(), &HttpMgr::sig_reset_mod_finish,
                this, &ResetDialog::slot_reset_mod_finish);

    initHandlers();
}

ResetDialog::~ResetDialog()
{
    delete ui;
}

bool ResetDialog::checkUser()
{
    QString user = ui->user_edit->text();
    if(user == ""){
        addTipErr(TipErr::TIP_USER_ERR, "用户名不能为空");
        return false;
    }

    QRegularExpression userRegex("^[a-zA-Z][a-zA-Z0-9_]{2,19}$");
    bool valid = userRegex.match(user).hasMatch();
    if(!valid){
        addTipErr(TipErr::TIP_USER_ERR, "用户名只能为字母、数字、下划线，长度3-20位");
        return false;
    }else{
        delTipErr(TipErr::TIP_USER_ERR);
    }
    return true;
}

bool ResetDialog::checkEmail()
{
    QString email = ui->email_edit->text();
    if(email == ""){
        addTipErr(TipErr::TIP_EMAIL_ERR, "邮箱不能为空");
        return false;
    }

    QRegularExpression regexp(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    bool match = regexp.match(email).hasMatch();
    if(!match){
        addTipErr(TipErr::TIP_EMAIL_ERR, "邮箱格式不正确");
        return false;
    }else{
        delTipErr(TipErr::TIP_EMAIL_ERR);
    }
    return true;
}

bool ResetDialog::checkNewPwd()
{
    QString pwd = ui->newPwd_edit->text();
    if(pwd == ""){
        addTipErr(TipErr::TIP_PWD_ERR, "密码不能为空");
        return false;
    }

    QRegularExpression passRegex("^(?=.*[A-Za-z])(?=.*\\d)[A-Za-z\\d@$!%*#?&]{6,}$");
    bool valid = passRegex.match(pwd).hasMatch();
    if(!valid){
        addTipErr(TipErr::TIP_PWD_ERR, "密码至少6位，包含字母和数字");
        return false;
    }else{
        delTipErr(TipErr::TIP_PWD_ERR);
    }
    return true;
}

bool ResetDialog::checkVerify()
{
    QString verify = ui->verify_edit->text();
    if(verify == ""){
        addTipErr(TipErr::TIP_VERIFY_ERR, "验证码不能为空");
        return false;
    }

    return true;
}

void ResetDialog::addTipErr(TipErr te, QString tip)
{
    _tips.insert(te, tip);
    showTip(_tips.first(), false);
}

void ResetDialog::delTipErr(TipErr te)
{
    _tips.remove(te);
    if(_tips.empty()){
        ui->tip_label->clear();
        return;
    }
    showTip(_tips.first(), false);
}

void ResetDialog::showTip(QString tip, bool result)
{
    ui->tip_label->setText(tip);
    if(result){
        ui->tip_label->setProperty("state", "normal");
    }else{
        ui->tip_label->setProperty("state", "err");
    }
    repolish(ui->tip_label);
}

void ResetDialog::initHandlers()
{
    _handlers[ReqId::ID_GET_VERIFY_CODE] = [this](const QJsonObject& json){
        auto error = json["error"].toInt();
        if(error != ErrorCodes::SUCCESS){
            showTip(tr("参数错误"), false);
            return;
        }

        showTip(get_verify_success, true);
        auto email = json["email"].toString();
        qDebug() << "Get verify code email: " << email;
    };

    _handlers[ReqId::ID_RESET_PWD] = [this](const QJsonObject& json){
        auto error = json["error"].toInt();
        if(error != ErrorCodes::SUCCESS){
            qDebug() << "Error: " << error;
            showTip(tr("参数错误"), false);
            return;
        }
        showTip(tr("修改密码成功"), true);
        auto user = json["user"].toString();
        auto email = json["email"].toString();
        qDebug() << "Reset pwd user: " << user;
        qDebug() << "Reset pwd email: " << email;
        ui->user_edit->clear();
        ui->email_edit->clear();
        ui->verify_edit->clear();
        ui->newPwd_edit->clear();
    };
}


void ResetDialog::on_get_verify_btn_clicked()
{
    bool valid = checkUser();
    valid = checkEmail();
    if(!valid){
        return;
    }

    QString email = ui->email_edit->text();

    QJsonObject json;
    json["email"] = email;
    HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix + "/get_verify"),
                                        json, ReqId::ID_GET_VERIFY_CODE, Modules::RESETMOD);
}


void ResetDialog::on_confirm_btn_clicked()
{
    bool valid = checkUser();
    if(!valid){
        return;
    }

    valid = checkEmail();
    if(!valid){
        return;
    }

    valid = checkVerify();
    if(!valid){
        return;
    }

    valid = checkNewPwd();
    if(!valid){
        return;
    }

    QString user = ui->user_edit->text();
    QString email = ui->email_edit->text();
    QString verifycode = ui->verify_edit->text();
    QString newPwd = ui->newPwd_edit->text();

    QJsonObject json;
    json["user"] = user;
    json["email"] = email;
    json["verifycode"] = verifycode;
    json["newPwd"] = xorString(newPwd);
    HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix + "/reset_pwd"),
                                        json, ReqId::ID_RESET_PWD, Modules::RESETMOD);
}

void ResetDialog::slot_reset_mod_finish(ReqId id, QString res, ErrorCodes err)
{
    if(err != ErrorCodes::SUCCESS){
        showTip(tr("网络请求错误"), false);
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(res.toUtf8());
    if(doc.isNull()){
        showTip(tr("json解析错误"), false);
        return;
    }

    if(!doc.isObject()){
        showTip(tr("json解析错误"), false);
        return;
    }

    qDebug() << "id: " << id;
    _handlers[id](doc.object());
}


void ResetDialog::on_return_btn_clicked()
{
    emit switchLogin();
}

