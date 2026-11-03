#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include <QDialog>
#include "clicklabel.h"
#include "chatuseritem.h"
#include <QRandomGenerator>
#include <vector>
#include "global.h"
#include "userdata.h"
#include <deque>
#include <QTimer>
#include "sqlmgr.h"
#include "toast.h"

/******************************************************************************
 *
 * @file            chatdialog.h
 * @brief           聊天界面类
 * @description
 *
 * @author          YuHaoNan
 * @date            2026/04/20
 * @history
 *****************************************************************************/
namespace Ui {
class ChatDialog;
}

class ChatDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChatDialog(std::vector<std::shared_ptr<ApplyInfo>> apply_list, QMap<int, std::shared_ptr<UserInfo>> chatUser_list,
                        qint64 unread_msg_count, QMap<qint64, int>, QWidget *parent = nullptr);
    ~ChatDialog();
    void setApplyList(const std::vector<std::shared_ptr<ApplyInfo>>& apply_list);
    QString getChatUserNick(int uid);
    QString getChatUserIcon(int uid);
protected:
    bool eventFilter(QObject* o, QEvent* e);
    void showEvent(QShowEvent *event) override;
private:
    void InitSideHeadIcon();
    void handleGlobalMousePress(QMouseEvent* e);
    void ShowSearch(bool bsearch);
    void addChatUserList();
    void addContactUserList();
    void LoadChatThread();
    void AddChatUser(std::shared_ptr<ChatThreadData>);
    void ChangeSideState();
    void UploadVideoCover(std::shared_ptr<VideoChatData> videoData);
    void UploadVideoBody(std::shared_ptr<VideoChatData> videoData);

    Ui::ChatDialog *ui;
    ChatUIMode _mode;
    ChatUIMode _state;
    bool _b_loading;
    short currentChatPageIndex;
    QTimer* _timer;
    Toast* _toast;
    qint64 _unread_msg_count;
    QMap<qint64, int> _unread_map;

signals:
    void sig_switch_apply_friend_page();
private slots:
    void slot_text_chat_msg(QVector<std::shared_ptr<ChatDataBase>>, int);
    void slot_load_chat_thread();
    void slot_load_chat_message(QVector<std::shared_ptr<ChatDataBase>>);
    void slot_add_chat_item(std::shared_ptr<ChatThreadData>);
    void slot_insert_chat_message(QVector<std::shared_ptr<ChatDataBase>>, qint64);
    void slot_reset_label_icon(const QString&);
    void slot_upload_img_chat_message(std::shared_ptr<ImgChatData>);
    void slot_download_file(qint64 msg_id, qint64 thread_id, QString md5, QString name, int send_uid);
    void slot_download_img_chat_data(qint64, std::shared_ptr<ChatDataBase>);
    void slot_notify_download_file(int send_uid, qint64 thread_id, qint64 msg_id, QString name, QString md5);
    void slot_upload_video_chat_message(std::shared_ptr<VideoChatData>);
    void slot_download_video_chat_msg(int send_uid, qint64 thread_id, qint64 msg_id, QString name, QString cover_name, int duration);
    void slot_download_video_cover(int send_uid, qint64 thread_id, qint64 msg_id, QString name);
    void slot_download_video_body(int send_uid, qint64 thread_id, qint64 msg_id, QString name);
    void slot_download_video_chat_data(qint64, std::shared_ptr<VideoChatData>);
};

#endif // CHATDIALOG_H
