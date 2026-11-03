#ifndef APPLYFRIENDPAGE_H
#define APPLYFRIENDPAGE_H

#include <QWidget>
#include "userdata.h"

class ApplyFriendPage : public QWidget
{
    Q_OBJECT
public:
    explicit ApplyFriendPage(QWidget *parent = nullptr);
    void addNewApply(std::shared_ptr<ApplyInfo> _apply_info);
signals:
    void sig_switch_default_page();
    void sig_add_chat_item(std::shared_ptr<ApplyInfo>);
private slots:
    void slot_notify_apply_friend(std::shared_ptr<AddFriendApply>);
};

#endif // APPLYFRIENDPAGE_H
