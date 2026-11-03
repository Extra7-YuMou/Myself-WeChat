#ifndef MESSAGEMODEL_H
#define MESSAGEMODEL_H

#include <QAbstractListModel>
#include <QObject>
#include "userdata.h"

class MessageModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        RoleMessageId = Qt::UserRole + 1,
        RoleContent,
        RoleSendUid,
        RoleMsgType,
        RoleThreadId,
        RoleImgTotalPath,
        RolePixmapSize,
        RoleVideoCoverPath,
        RoleVideoDuration,
        RoleVideoPath,
    };

    explicit MessageModel(QObject *parent = nullptr);
    void setMessages(const QVector<std::shared_ptr<ChatDataBase>>& msg_list);
    void appendMessage(const std::shared_ptr<ChatDataBase> msg);
    void replaceOrAppend(const std::shared_ptr<ChatDataBase> msg);
    void setHeadIcon();
    int rowCount(const QModelIndex& index = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;

private:
    QVector<std::shared_ptr<ChatDataBase>> _msg_list;
};

#endif // MESSAGEMODEL_H
