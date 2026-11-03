#include "contactuseritem.h"
#include "ui_contactuseritem.h"

ContactUserItem::ContactUserItem(QWidget *parent)
    : ListItemBase(parent)
    , ui(new Ui::ContactUserItem)
{
    ui->setupUi(this);

    setItemType(ListItemType::CONTACT_USER_ITEM);
}

ContactUserItem::~ContactUserItem()
{
    delete ui;
}

void ContactUserItem::setInfo(int uid, QString nick, QString icon)
{
    _info = std::make_shared<UserInfo>();
    _info->_uid = uid;
    _info->_nick = nick;
    _info->_icon = icon;

    QPixmap pix(_info->_icon);

    pix = pix.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->icon_lb->setPixmap(pix);
    ui->icon_lb->setScaledContents(true);

    ui->nick_lb->setText(nick);
}
