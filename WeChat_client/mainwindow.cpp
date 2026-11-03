#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "tcpmgr.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow), _login_dlg(nullptr), _chat_dlg(nullptr), _reg_dlg(nullptr), _reset_dlg(nullptr)
{
    ui->setupUi(this);

    // 创建一个CentralWidget，并将其设置为MainWindow的中心组件
    _login_dlg = new LoginDialog(this);
    _login_dlg->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    _login_dlg->show();
    setCentralWidget(_login_dlg);

    _reg_dlg = new RegisterDialog();
    connect(_login_dlg, &LoginDialog::switchRegister, this, &MainWindow::slotSwitchRegister);
    connect(_reg_dlg, &RegisterDialog::switchLogin, this, &MainWindow::slotSwitchLogin);
    connect(_reg_dlg, &RegisterDialog::switchLogin2, this, &MainWindow::slotSwitchLogin2);
    connect(_login_dlg, &LoginDialog::switchReset, this, &MainWindow::slotSwitchReset);
    connect(TcpMgr::GetInstance().get(), &TcpMgr::switchChat, this, &MainWindow::slotSwitchChat);
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_notify_off_line, this, &MainWindow::slotSwitchLogin4);

    //emit TcpMgr::GetInstance()->switchChat();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::slotSwitchRegister()
{
    _reg_dlg = new RegisterDialog(this);
    _reg_dlg->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(_reg_dlg);

    _login_dlg->hide();
    _reg_dlg->show();
    // 连接注册界面返回登录登录信号
    connect(_reg_dlg, &RegisterDialog::switchLogin, this, &MainWindow::slotSwitchLogin);
}

// 注册界面返回
void MainWindow::slotSwitchLogin()
{
    // 创建一个CentralWidget，并将其设置为MainWindow的中心组件
    _login_dlg = new LoginDialog(this);
    _login_dlg->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(_login_dlg);

    _reg_dlg->hide();
    _login_dlg->show();
    // 连接登陆界面注册信号
    connect(_login_dlg, &LoginDialog::switchRegister, this, &MainWindow::slotSwitchRegister);
    connect(_login_dlg, &LoginDialog::switchReset, this, &MainWindow::slotSwitchReset);
}

// 注册成功界面返回
void MainWindow::slotSwitchLogin2()
{
    _login_dlg = new LoginDialog(this);
    _login_dlg->setWindowFlags(Qt::FramelessWindowHint | Qt::CustomizeWindowHint);
    setCentralWidget(_login_dlg);
    _reg_dlg->hide();
    _login_dlg->show();
    connect(_login_dlg, &LoginDialog::switchRegister, this, &MainWindow::slotSwitchRegister);
    connect(_login_dlg, &LoginDialog::switchReset, this, &MainWindow::slotSwitchReset);
}

void MainWindow::slotSwitchReset()
{
    _reset_dlg = new ResetDialog(this);
    _reset_dlg->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(_reset_dlg);

    _login_dlg->hide();
    _reset_dlg->show();
    connect(_reset_dlg, &ResetDialog::switchLogin, this, &MainWindow::slotSwitchLogin3);
}

// 重置密码界面返回
void MainWindow::slotSwitchLogin3()
{
    _login_dlg = new LoginDialog(this);
    _login_dlg->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(_login_dlg);

    _reset_dlg->hide();
    _login_dlg->show();
    connect(_login_dlg, &LoginDialog::switchRegister, this, &MainWindow::slotSwitchRegister);
    connect(_login_dlg, &LoginDialog::switchReset, this, &MainWindow::slotSwitchReset);
}

void MainWindow::slotSwitchChat(std::vector<std::shared_ptr<ApplyInfo>> apply_list, QMap<int, std::shared_ptr<UserInfo>> chatUser_list,
                    qint64 unread_msg_count, QMap<qint64, int> unread_map){
    _chat_dlg = new ChatDialog(apply_list, chatUser_list, unread_msg_count, unread_map, this);
    _chat_dlg->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(_chat_dlg);

    _login_dlg->hide();
    _chat_dlg->show();
    _chat_dlg->setMinimumSize(QSize(950, 600));
    _chat_dlg->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
}

// 聊天界面被顶号返回
void MainWindow::slotSwitchLogin4(int uid)
{
    QMessageBox::information(nullptr, "通知下线", "该账号在其他地方登录，即将下线");
    qDebug() << "下线uid: " << uid;
    _login_dlg = new LoginDialog(this);
    _login_dlg->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(_login_dlg);

    _chat_dlg->hide();
    _login_dlg->show();
    this->setFixedSize(QSize(300, 500));
    connect(_login_dlg, &LoginDialog::switchRegister, this, &MainWindow::slotSwitchRegister);
    connect(_login_dlg, &LoginDialog::switchReset, this, &MainWindow::slotSwitchReset);
}
