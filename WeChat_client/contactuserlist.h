#ifndef CONTACTUSERLIST_H
#define CONTACTUSERLIST_H

#include <QListWidget>
#include "contactuseritem.h"

class ContactUserList : public QListWidget
{
    Q_OBJECT
public:
    ContactUserList(QWidget* parent = nullptr);
    void addContactUsers();

protected:
    bool eventFilter(QObject* o, QEvent* e);
private:
    ContactUserItem* _add_friend_item;

private slots:
    void slot_item_clicked(QListWidgetItem *item);

signals:
    void sig_switch_apply_friend_page();
    void sig_switch_friend_info_page();
};

#endif // CONTACTUSERLIST_H
