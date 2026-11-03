#ifndef APPLYFRIENDITEM_H
#define APPLYFRIENDITEM_H

#include <QWidget>
#include "ListItemBase.h"
#include "userdata.h"

namespace Ui {
class ApplyFriendItem;
}

class ApplyFriendItem : public ListItemBase
{
    Q_OBJECT

public:
    explicit ApplyFriendItem(QWidget *parent = nullptr);
    ~ApplyFriendItem();
    int getUid();
    void setInfo(std::shared_ptr<ApplyInfo> _apply_info);
    int getHeight(){return 88;}
    void setStatus(bool added);
    void setIndex(int index);
private:
    Ui::ApplyFriendItem *ui;
    std::shared_ptr<ApplyInfo> _apply_info;
    bool _added;

signals:
    void sig_auth_apply(std::shared_ptr<ApplyInfo> _apply_info);
private slots:
    void on_add_friend_btn_clicked();
};

#endif // APPLYFRIENDITEM_H
