#ifndef CHATUSERLIST_H
#define CHATUSERLIST_H

#include <QListWidget>
#include "userdata.h"
#include <deque>

/******************************************************************************
 *
 * @file            chatuserlist.h
 * @brief           聊天用户列表
 * @description
 *
 * @author          YuHaoNan
 * @date            2026/04/20
 * @history
 *****************************************************************************/
class ChatUserList : public QListWidget
{
    Q_OBJECT
public:
    ChatUserList(QWidget* parent = nullptr);
private:
    bool eventFilter(QObject* watched, QEvent* e);
signals:
};

#endif // CHATUSERLIST_H
