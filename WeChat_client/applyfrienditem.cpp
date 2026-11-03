#include "applyfrienditem.h"
#include "ui_applyfrienditem.h"
#include <QDir>
#include "usermgr.h"
#include "tcpmgr.h"

ApplyFriendItem::ApplyFriendItem(QWidget *parent)
    : ListItemBase(parent), _added(false)
    , ui(new Ui::ApplyFriendItem)
{
    ui->setupUi(this);
    setItemType(ListItemType::APPLY_FRIEND_ITEM);
    ui->add_friend_btn->setState("normal", "hover", "press");
    ui->add_friend_btn->hide();

    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_auth_friend_success, [this](){
        setStatus(true);
    });
}

ApplyFriendItem::~ApplyFriendItem()
{
    delete ui;
}

int ApplyFriendItem::getUid()
{
    return _apply_info->_uid;
}

void ApplyFriendItem::setInfo(std::shared_ptr<ApplyInfo> info)
{
    this->_apply_info = info;

    QString appPath = QCoreApplication::applicationDirPath();
    QString iconPath = QDir::toNativeSeparators(appPath + QDir::separator()
                                                + "static" + QDir::separator() + info->_icon);
    QPixmap pix(iconPath);
    ui->icon_lb->setPixmap(pix.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);

    ui->nick_lb->setText(info->_nick);
    ui->chat_lb->setText(info->_desc);
}

void ApplyFriendItem::setStatus(bool added)
{
    if(added){
        ui->add_friend_btn->hide();
        ui->already_add_lb->show();
    }else{
        ui->add_friend_btn->show();
        ui->already_add_lb->hide();
    }

    _added = added;
}

void ApplyFriendItem::on_add_friend_btn_clicked()
{
    QJsonObject json;
    json["self_uid"] = UserMgr::GetInstance()->getUid();
    json["friend_uid"] = _apply_info->_uid;
    json["desc"] = ui->chat_lb->text();
    qDebug() << "json: " << json;

    QJsonDocument doc(json);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_AUTH_FRIEND_REQ,
                                              data);
}