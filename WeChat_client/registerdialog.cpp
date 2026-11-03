#include "registerdialog.h"
#include "ui_registerdialog.h"
#include "Global.h"
#include <QRegularExpression>
#include "HttpMgr.h"

RegisterDialog::RegisterDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RegisterDialog), _count(5)
{
    ui->setupUi(this);
    ui->pass_edit->setEchoMode(QLineEdit::Password);
    ui->confirm_edit->setEchoMode(QLineEdit::Password);
    ui->tip_label->setProperty("state", "normal");
    repolish(ui->tip_label);

    ui->pass_visible->setState("unvisible", "unvisible_hover", "",
                               "visible", "visible_hover", "");
    ui->pass_visible->setCursor(Qt::PointingHandCursor);

    ui->confirm_visible->setState("unvisible", "unvisible_hover", "",
                               "visible", "visible_hover", "");
    ui->confirm_visible->setCursor(Qt::PointingHandCursor);

    initHandlers();

    connect(HttpMgr::GetInstance().get(), &HttpMgr::sig_reg_mod_finish, this, &RegisterDialog::slot_reg_mod_finish);

    connect(ui->user_edit, &QLineEdit::editingFinished, [this](){
        checkUser();
    });

    connect(ui->email_edit, &QLineEdit::editingFinished, [this](){
        checkEmail();
    });

    connect(ui->pass_edit, &QLineEdit::editingFinished, [this](){
        checkPwd();
    });

    connect(ui->confirm_edit, &QLineEdit::editingFinished, [this](){
        checkConfirm();
    });

    connect(ui->verify_edit, &QLineEdit::editingFinished, [this](){
        checkVerify();
    });

    connect(ui->pass_visible, &ClickLabel::clicked, [this](){
        auto state = ui->pass_visible->GetState();
        if(state == ClickState::normal){
            ui->pass_edit->setEchoMode(QLineEdit::Password);
        }else{
            ui->pass_edit->setEchoMode(QLineEdit::Normal);
        }
    });

    connect(ui->confirm_visible, &ClickLabel::clicked, [this](){
        auto state = ui->confirm_visible->GetState();
        if(state == ClickState::normal){
            ui->confirm_edit->setEchoMode(QLineEdit::Password);
        }else{
            ui->confirm_edit->setEchoMode(QLineEdit::Normal);
        }
    });

    _timer = new QTimer(this);
    ui->finish_label->setText("注册成功，5秒后返回。\n你也可以点击返回登录按钮返回登陆界面。");
    connect(_timer, &QTimer::timeout, [this](){
        if(_count == 0){
            _timer->stop();
            emit switchLogin2();
        }
        QString tips = tr("注册成功，%1秒后返回。\n你也可以点击返回登录按钮返回登陆界面。").arg(_count);
        --_count;
        ui->finish_label->setText(tips);
    });
}

RegisterDialog::~RegisterDialog()
{
    delete ui;
}

void RegisterDialog::on_get_verify_btn_clicked()
{
    QString email = ui->email_edit->text();
    QRegularExpression regexp(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    bool match = regexp.match(email).hasMatch();
    if(match){
        QJsonObject jsonObj;
        jsonObj["email"] = email;
        HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix + "/get_verify"),
                                            jsonObj, ReqId::ID_GET_VERIFY_CODE, Modules::REGISTERMOD);
    }else{
        showTip(tr("邮箱格式错误，请确认后输入!"), false);
    }
}

void RegisterDialog::slot_reg_mod_finish(ReqId id, QString res, ErrorCodes err)
{
    if(err != ErrorCodes::SUCCESS){
        showTip(tr("网络请求错误"), false);
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(res.toUtf8());
    // 解析doc
    if(doc.isNull()){
        showTip(tr("json解析错误"), false);
        return;
    }

    if(!doc.isObject()){
        showTip(tr("json解析错误"), false);
        return;
    }

    _handlers[id](doc.object());
    return;
}

void RegisterDialog::showTip(QString tip, bool result)
{
    ui->tip_label->setText(tip);
    if(result){
        ui->tip_label->setProperty("state", "normal");
    }else{
        ui->tip_label->setProperty("state", "err");
    }
    repolish(ui->tip_label);
}

void RegisterDialog::initHandlers()
{
    _handlers[ReqId::ID_GET_VERIFY_CODE] = [this](const QJsonObject& jsonObj){
        auto err = jsonObj["error"].toInt();
        if(err != ErrorCodes::SUCCESS){
            showTip(tr("参数错误"), false);
            return;
        }

        auto email = jsonObj["email"].toString();
        qDebug() << "email is " << email;
        showTip(get_verify_success, true);
    };

    _handlers[ReqId::ID_REG_USER] = [this](const QJsonObject& json){
        auto err = json["error"].toInt();
        if(err != ErrorCodes::SUCCESS){
            showTip(tr("参数错误"), false);
            return;
        }

        showTip(tr("注册成功"), true);
        qDebug() << "email: " << json["email"].toString();
        qDebug() << "uuid: " << json["uuid"].toString();
        changeTipPage();
        return;
    };
}

bool RegisterDialog::checkUser(){
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

bool RegisterDialog::checkEmail(){
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

bool RegisterDialog::checkPwd()
{
    QString pwd = ui->pass_edit->text();
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

bool RegisterDialog::checkConfirm(){
    QString confirm = ui->confirm_edit->text();
    if(confirm == ""){
        addTipErr(TipErr::TIP_CONFIRM_ERR, "确认密码不能为空");
        return false;
    }

    QString pwd = ui->pass_edit->text();
    if(confirm == pwd && checkPwd()){
        delTipErr(TipErr::TIP_PWD_CONFIRM);
    }else{
        addTipErr(TipErr::TIP_PWD_CONFIRM, "密码和确认密码不一致，请重新确认");
        return false;
    }

    return true;
}

bool RegisterDialog::checkVerify(){
    QString verify = ui->verify_edit->text();
    if(verify == ""){
        addTipErr(TipErr::TIP_VERIFY_ERR, "验证码不能为空");
        return false;
    }

    return true;
}

void RegisterDialog::addTipErr(TipErr te, QString tip){
    _tips.insert(te, tip);
    showTip(_tips.first(), false);
}

void RegisterDialog::delTipErr(TipErr te){
    _tips.remove(te);
    if(_tips.empty()){
        ui->tip_label->clear();
        return;
    }
    showTip(_tips.first(), false);
}

void RegisterDialog::changeTipPage(){
    ui->stackedWidget->setCurrentWidget(ui->page2);
    _timer->start(1000);
}

void RegisterDialog::on_cancel_btn_clicked()
{
    emit switchLogin();
}

void RegisterDialog::on_confirm_btn_clicked()
{
    bool valid = checkUser();
    if(!valid){
        return;
    }

    valid = checkEmail();
    if(!valid){
        return;
    }

    valid = checkPwd();
    if(!valid){
        return;
    }

    valid = checkConfirm();
    if(!valid){
        return;
    }

    valid = checkVerify();
    if(!valid){
        return;
    }

    QString user = ui->user_edit->text();
    QString email = ui->email_edit->text();
    QString passwd = ui->email_edit->text();
    QString confirm = ui->confirm_edit->text();
    QString verifycode = ui->verify_edit->text();

    QJsonObject json;
    json["user"] = user;
    json["email"] = email;
    json["passwd"] = xorString(passwd);
    json["confirm"] = xorString(passwd);
    json["verifycode"] = verifycode;
    HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix + "/user_register"),
                                        json, ReqId::ID_REG_USER, Modules::REGISTERMOD);
}


void RegisterDialog::on_pushButton_clicked()
{
    emit switchLogin2();
}

