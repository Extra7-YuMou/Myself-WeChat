#include "findresultdialog.h"
#include "ui_findresultdialog.h"
#include <QDir>
#include "applyfrienddialog.h"
#include <QMouseEvent>
#include <QDebug>

FindResultDialog::FindResultDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FindResultDialog)
{
    ui->setupUi(this);

    setWindowTitle("添加好友");
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

    // QString appPath = QCoreApplication::applicationDirPath();
    // QString head_path = QDir::toNativeSeparators(appPath + QDir::separator() +
    //                                              "static" + QDir::separator() + "head_1.jpg");
    // QPixmap headIcon(head_path);
    // headIcon = headIcon.scaled(ui->head_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    // ui->head_lb->setPixmap(headIcon);
    ui->add_user_btn->setState("normal", "hover", "press");
    this->setModal(true);
}

FindResultDialog::~FindResultDialog()
{
    delete ui;
}

void FindResultDialog::setSearchInfo(std::shared_ptr<SearchInfo> si)
{
    _si = si;
    ui->nick_lb->setText(si->_nick);

    QString appPath = QCoreApplication::applicationDirPath();
    QString iconPath = QDir::toNativeSeparators(appPath + QDir::separator() +
                                                "static" + QDir::separator() + si->_icon);
    QPixmap pix(iconPath);
    ui->head_lb->setPixmap(pix.scaled(ui->head_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->head_lb->setScaledContents(true);
}

void FindResultDialog::setAddBtnText()
{
    ui->add_user_btn->setText("已添加好友");
    ui->add_user_btn->setDisabled(true);
}

void FindResultDialog::mousePressEvent(QMouseEvent *e)
{
    if(e->type() == QMouseEvent::MouseButtonPress && e->button() == Qt::LeftButton){
        offset = e->globalPos() - this->pos();
    }

    QDialog::mousePressEvent(e);
}

void FindResultDialog::mouseMoveEvent(QMouseEvent *e)
{
    if(e->type() == QMouseEvent::MouseMove){
        QPoint movePos = e->globalPos() - offset;
        this->move(movePos);
    }
}

void FindResultDialog::on_add_user_btn_clicked()
{
    ApplyFriendDialog* applyFriendDlg = new ApplyFriendDialog;
    applyFriendDlg->setSearchInfo(_si);
    applyFriendDlg->setModal(true);
    applyFriendDlg->show();

    this->close();
}


