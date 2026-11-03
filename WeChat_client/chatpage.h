#ifndef CHATPAGE_H
#define CHATPAGE_H

#include <QStackedWidget>
#include "userdata.h"
#include <QDir>
#include <QStandardPaths>
#include "imagecropperdialog.h"
#include "imagecropperlabel.h"
#include <QFileDialog>
#include "messagemodel.h"
#include "messagedelegate.h"
#include <QListview>

namespace Ui {
class ChatPage;
}

/******************************************************************************
 *
 * @file            chatpage.h
 * @brief           聊天界面
 * @description
 *
 * @author          YuHaoNan
 * @date            2026/04/20
 * @history
 *****************************************************************************/
class ChatPage : public QStackedWidget
{
    Q_OBJECT

public:
    explicit ChatPage(QWidget *parent = nullptr);
    ~ChatPage();
    void loadFriendApply(const std::vector<std::shared_ptr<ApplyInfo>>& apply_list);
    void addTextChat(std::shared_ptr<ChatDataBase>);
    void ResetHeadIcon();
    void upsertImgMessage(std::shared_ptr<ChatDataBase> img_data);
    void upsertVideoMessage(std::shared_ptr<VideoChatData> videoData);
    qint64 GetThreadId() const;
public slots:
    void slot_loading_data(std::shared_ptr<UserInfo> _ui, std::shared_ptr<ChatThreadData>);
    void slot_appendFile(const QString& filePath);
    void slot_loading_contactUser(std::shared_ptr<UserInfo>);
    void slot_switch_setting();
    void slot_load_chat_message(QVector<std::shared_ptr<ChatDataBase>>);
    //void sig_add_chat_item(std::shared_ptr<ApplyInfo>);

private slots:
    void on_send_btn_clicked();

    void on_upload_head_btn_clicked();

    void on_setting_submit_btn_clicked();
    void on_message_click(const QModelIndex& index);

signals:
    void sig_update_head_icon();
    void sig_insert_chat_message(QVector<std::shared_ptr<ChatDataBase>>, qint64);
    void sig_request_download_img(qint64 msg_id, qint64 thread_id, QString name, int send_uid);
    void sig_request_download_video(qint64 msg_id, qint64 thread_id, QString name, int send_uid);
private:
    bool ExistsFile(qint64 tid, QString fileName);
    bool ExistsVideoCover(QString videoName);
    std::shared_ptr<ChatDataBase> EnsureDisplayMsg(std::shared_ptr<ChatDataBase> data);
    void RequestMissingImg(std::shared_ptr<ChatDataBase> data);

    Ui::ChatPage *ui;

    std::shared_ptr<UserInfo> _user_info;
    std::shared_ptr<ChatThreadData> _chat_data;
    MessageModel* m_msgModel;
    MessageDelegate* m_msgDelegate;
};

#endif // CHATPAGE_H
