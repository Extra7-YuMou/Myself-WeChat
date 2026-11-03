#ifndef CHATUSERITEM_H
#define CHATUSERITEM_H

#include <QWidget>
#include "listitembase.h"
#include <QMouseEvent>
#include "userdata.h"

namespace Ui {
class ChatUserItem;
}

/******************************************************************************
 *
 * @file            chatuseritem.h
 * @brief           聊天用户项目
 * @description
 *
 * @author          YuHaoNan
 * @date            2026/04/20
 * @history
 *****************************************************************************/
class ChatUserItem : public ListItemBase
{
    Q_OBJECT
public:
    explicit ChatUserItem(QWidget *parent = nullptr);
    ~ChatUserItem();

    QSize sizeHint() const override{
        return QSize(250, 60);
    }
    void setInfo(std::shared_ptr<ChatThreadData>);
    QString getNick();
    int GetThreadId(){return _chat_data->GetThreadId();}
    void setUnReadCount(int UnReadCount);
    void setLastMsg(const QString& msg);
    void SetChatData(std::shared_ptr<ChatThreadData>);
    void AddChatMessage(QVector<std::shared_ptr<ChatDataBase>>);
    std::shared_ptr<ChatThreadData> GetChatData();
protected:
    virtual void mousePressEvent(QMouseEvent* e);
private:
    Ui::ChatUserItem *ui;
    int _UnReadCount;
    std::shared_ptr<ChatThreadData> _chat_data;

signals:
    void sig_loading_data(std::shared_ptr<UserInfo>, std::shared_ptr<ChatThreadData>);
};
#endif // CHATUSERITEM_H
