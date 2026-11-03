#ifndef USERMGR_H
#define USERMGR_H
#include <QObject>
#include "Singleton.h"
#include <memory>
#include "userdata.h"
#include <QListWidgetItem>
#include <deque>
#include <mutex>
#include <QFileInfo>
#include <QStandardPaths>
#include <QLabel>

/******************************************************************************
 *
 * @file            usermgr.h
 * @brief           用户管理类
 * @description     用于管理用户信息
 *
 * @author          YuHaoNan
 * @date            2026/04/19
 * @history
 *****************************************************************************/
class UserMgr : public QObject, public Singleton<UserMgr>{
    Q_OBJECT
    friend class Singleton<UserMgr>;
public:
    ~UserMgr() = default;
    void setUid(const int& uid);
    void setName(const QString& name);
    void setToken(const QString& token);
    void setNick(const QString& nick);
    void setIcon(const QString& icon);
    void setDesc(const QString& desc);
    void setSex(const int& sex);

    QMap<int, std::shared_ptr<AddFriendApply>>& getFriendApplys();
    std::vector<std::shared_ptr<ApplyInfo>>& getApplyList();
    QMap<int, QListWidgetItem*>& getChatItemAdded();
    void addChatThread(std::shared_ptr<ChatThreadInfo>);
    QVector<std::shared_ptr<ChatThreadData>>& GetChatThreadList();
    QMap<int, std::shared_ptr<UserInfo>>& GetFriendList();
    std::shared_ptr<UserInfo> GetSelfInfo();
    void AddFileInfoByMd5(const QString& md5, std::shared_ptr<QFileInfo> fileInfo);
    std::shared_ptr<QFileInfo> GetFileInfoByMd5(const QString& md5);
    bool IsDownLoad(const QString& name);
    void AddDownLoadFile(const QString& name, std::shared_ptr<FileInfo>, const QString& dir_name = "");
    std::shared_ptr<FileInfo> GetDownLoadFile(const QString& name);
    void RmvDownLoadFile(const QString& name);
    void AddResetLabel(const QString& name, QLabel* label);
    QLabel* GetResetLabel(const QString& name);
    void RmvResetLabel(const QString& name);

    int getUid();
    QString getName();
    QString getToken();
    QString getNick();
    QString getIcon();
    QString getDesc();
    int GetSex();
    QString GetPathPrefix();
private:
    QString PATH_PREFIX;

    UserMgr();

    std::mutex _mtx;
    int _uid;
    QString _name;
    QString _token;
    QString _nick;
    QString _icon;
    QString _desc;
    int _sex;

    // ChatDialog中移过来的
    QMap<int, std::shared_ptr<AddFriendApply>> _friend_applys;
    std::vector<std::shared_ptr<ApplyInfo>> _apply_list;
    QMap<int, QListWidgetItem*> _chat_item_added;
    QVector<std::shared_ptr<ChatThreadInfo>> _chat_threads_info;
    QVector<std::shared_ptr<ChatThreadData>> _chat_threads;
    QMap<int, std::shared_ptr<UserInfo>> _friend_list;
    QMap<QString, std::shared_ptr<QFileInfo>> _md5_files;
    QMap<QString, std::shared_ptr<FileInfo>> _name_to_download_info;
    QMap<QString, QLabel*> _name_to_reset_label;
};

#endif // USERMGR_H
