#include "chatitembase.h"
#include <QGridLayout>
#include <QDir>
#include <QCoreApplication>

ChatItemBase::ChatItemBase(ChatRole role, QWidget *parent)
    : QWidget{parent}, m_role(role), _statusSize(16, 16), _status(MsgStatus::UNREAD)
{
    m_pStatusLabel = new QLabel;
    m_pNickLabel = new QLabel;
    m_pNickLabel->setObjectName("chat_user_nick");
    QFont font("Microsoft YaHei");
    font.setPixelSize(11);
    m_pNickLabel->setFont(font);
    m_pNickLabel->setMaximumHeight(20);
    m_pIconLabel = new QLabel;
    m_pIconLabel->setScaledContents(true);
    m_pIconLabel->setFixedSize(42, 42);
    m_pBubble = new QWidget();
    QGridLayout* layout = new QGridLayout;
    layout->setVerticalSpacing(3);
    layout->setHorizontalSpacing(3);
    layout->setContentsMargins(3,3,3,3);
    QSpacerItem* spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    if(m_role == ChatRole::Self){
        QPixmap pix(":/images/un_read.png");
        pix = pix.scaled(_statusSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_pStatusLabel->setPixmap(pix);
        m_pStatusLabel->installEventFilter(this);
        m_pNickLabel->setContentsMargins(0, 0, 8, 0);
        m_pNickLabel->setAlignment(Qt::AlignRight);
        layout->addWidget(m_pNickLabel, 0, 2, 1, 1);
        layout->addWidget(m_pIconLabel, 0, 3, 2, 1, Qt::AlignTop);
        layout->addWidget(m_pStatusLabel, 1, 1, 1, 1);
        layout->addWidget(m_pBubble, 1, 2, 1, 1);
        layout->addItem(spacer, 1, 0, 1, 1);
        // 设置行比例
        layout->setColumnStretch(0, 2);
        layout->setColumnStretch(1, 0);  // status 图标 (固定大小)
        layout->setColumnStretch(2, 3);  // 名字 + 气泡 (主要拉伸区域)
        layout->setColumnStretch(3, 0);  // 头像 (固定大小)
    }else{
        m_pNickLabel->setContentsMargins(8, 0, 0, 0);
        m_pNickLabel->setAlignment(Qt::AlignLeft);
        layout->addWidget(m_pIconLabel, 0, 0, 2, 1, Qt::AlignTop);
        layout->addWidget(m_pNickLabel, 0, 1, 1, 1);
        layout->addWidget(m_pBubble, 1, 1, 1, 1);
        layout->addItem(spacer, 1, 2, 1, 1);
        layout->setColumnStretch(2, 2);
        layout->setColumnStretch(1, 3);
    }
    this->setLayout(layout);
}

void ChatItemBase::setNick(const QString &nick)
{
    m_pNickLabel->setText(nick);
}

void ChatItemBase::setIcon(const QString &icon)
{
    QString appPath = QCoreApplication::applicationDirPath();
    QString iconPath = QDir::toNativeSeparators(appPath + QDir::separator() +
                                                "static" + QDir::separator() + icon);
    QPixmap pix(iconPath);
    m_pIconLabel->setPixmap(pix.scaled(m_pIconLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void ChatItemBase::setBubble(QWidget *bubble)
{
    auto* layout = (qobject_cast<QGridLayout*>)(this->layout());
    layout->replaceWidget(m_pBubble, bubble);
    delete m_pBubble;
    m_pBubble = bubble;
}

void ChatItemBase::setStatus(MsgStatus status)
{
    if(status == MsgStatus::UNREAD){
        m_pStatusLabel->setPixmap(QPixmap(":/images/un_read.png").scaled(_statusSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    if(status == MsgStatus::SENDFAIL){
        m_pStatusLabel->setPixmap(QPixmap(":/images/send_fail.png").scaled(_statusSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    if(status == MsgStatus::READED){
        m_pStatusLabel->setPixmap(QPixmap(":/images/readed.png").scaled(_statusSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    _status = status;
}

bool ChatItemBase::eventFilter(QObject *o, QEvent *e)
{
    if(o == m_pStatusLabel && e->type() == QEvent::MouseButtonPress){
        if(_status == MsgStatus::SENDFAIL){
            // 重新尝试发送消息的逻辑
        }
    }
    return QWidget::eventFilter(o, e);
}
;