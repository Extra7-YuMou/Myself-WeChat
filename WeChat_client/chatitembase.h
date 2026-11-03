#ifndef CHATITEMBASE_H
#define CHATITEMBASE_H
#include <QWidget>
#include <QLabel>
#include "global.h"

/******************************************************************************
 *
 * @file            chatitembase.h
 * @brief           聊天界面里的用户聊天信息框
 * @description
 *
 * @author          YuHaoNan
 * @date            2026/04/21
 * @history
 *****************************************************************************/
class ChatItemBase : public QWidget
{
    Q_OBJECT
public:
    explicit ChatItemBase(ChatRole role, QWidget *parent = nullptr);
    void setNick(const QString& nick);
    void setIcon(const QString& icon);
    void setBubble(QWidget* bubble);
    void setStatus(MsgStatus status);

protected:
    bool eventFilter(QObject* o, QEvent* e) override;

private:
    QLabel* m_pNickLabel;
    QLabel* m_pIconLabel;
    // 消息体
    QWidget* m_pBubble;
    // 状态
    QLabel* m_pStatusLabel;

    ChatRole m_role;
    QSize _statusSize;
    MsgStatus _status;
signals:
};

#endif // CHATITEMBASE_H
