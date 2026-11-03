#include "chatdialog.h"
#include "ui_chatdialog.h"
#include "tcpmgr.h"
#include "usermgr.h"
#include <QMessageBox>
#include "filetcpmgr.h"
#include <QCryptographicHash>
#include <QFile>

ChatDialog::ChatDialog(std::vector<std::shared_ptr<ApplyInfo>> apply_list, QMap<int, std::shared_ptr<UserInfo>> chatUser_list,
                       qint64 unread_msg_count, QMap<qint64, int> unread_map, QWidget *parent)
    : QDialog(parent), _toast(nullptr), _unread_map(unread_map), _unread_msg_count(unread_msg_count)
    , ui(new Ui::ChatDialog), _mode(ChatUIMode::ChatMode), _state(ChatUIMode::ChatMode), _b_loading(false),
    currentChatPageIndex(0)
{
    UserMgr::GetInstance()->getApplyList() = apply_list;
    UserMgr::GetInstance()->GetFriendList() = chatUser_list;

    ui->setupUi(this);
    ui->search_list->hide();
    ui->con_user_list->hide();
    connect(ui->con_user_list, &ContactUserList::sig_switch_apply_friend_page, [this](){
        ui->chat_data_wid->setCurrentIndex(ChatDataWidType::ApplyFriend);
    });

    ui->side_chat_lb->setState("normal", "hover", "press",
                                "select", "select_hover", "select_press");
    ui->side_chat_lb->setProperty("state", "press");
    repolish(ui->side_chat_lb);
    connect(ui->side_chat_lb, &ClickLabel::clicked, [this](){
        _mode = ChatUIMode::ChatMode;
        ChangeSideState();

        ui->chat_user_list->show();
        ui->con_user_list->hide();
        ui->search_list->hide();
    });
    ui->side_chat_lb->setType(SideType::Chat);
    ui->side_chat_lb->setUnReadCount(unread_msg_count);

    ui->side_contact_lb->setState("normal", "hover", "press",
                                  "select", "select_hover", "select_press");
    connect(ui->side_contact_lb, &ClickLabel::clicked, [this](){
        _mode = ChatUIMode::ContactMode;
        ChangeSideState();

        ui->chat_user_list->hide();
        ui->con_user_list->show();
        ui->search_list->hide();
    });
    ui->side_contact_lb->setType(SideType::Contact);

    ui->side_setting_lb->setState("normal", "hover", "press",
                                    "select", "select_hover", "select_press");
    connect(ui->side_setting_lb, &ClickLabel::clicked, [this](){
        _mode = ChatUIMode::SettingsMode;
        ChangeSideState();

        ui->chat_user_list->hide();
        ui->con_user_list->hide();
        ui->search_list->show();
    });
    connect(ui->side_setting_lb, &ClickLabel::clicked, ui->chat_data_wid, &ChatPage::slot_switch_setting);
    ui->side_setting_lb->setType(SideType::Setting);

    ui->add_btn->setState("normal", "hover", "press");
    connect(ui->add_btn, &ClickButton::clicked, this, [this](){
        auto search = ui->search_edit->text();
        QRegularExpression re(QStringLiteral("^[0-9]+$"));
        if(re.match(search).hasMatch()){
            auto uid = search.toInt();

            QJsonObject json;
            json["uid"] = uid;
            QByteArray data = QJsonDocument(json).toJson(QJsonDocument::Compact);
            emit TcpMgr::GetInstance().get()->sig_send_data(ReqId::ID_SEARCH_USER_REQ, data);
        }
    });

    QAction* searchAction = new QAction(ui->search_edit);
    searchAction->setIcon(QIcon(":/images/search.png"));
    ui->search_edit->addAction(searchAction, QLineEdit::LeadingPosition);
    ui->search_edit->setPlaceholderText("搜索");

    QAction* searchClear = new QAction(ui->search_edit);
    searchClear->setIcon(QIcon(":/images/close_transparent.png"));
    ui->search_edit->addAction(searchClear, QLineEdit::TrailingPosition);
    connect(ui->search_edit, &QLineEdit::textChanged, [this, searchClear](){
        if(ui->search_edit->text().length() != 0){
            searchClear->setIcon(QIcon(":/images/close_search.png"));
        }else{
            searchClear->setIcon(QIcon(":/images/close_transparent.png"));
        }
    });

    connect(searchClear, &QAction::triggered, [this, searchClear](){
        if(ui->search_edit->text().length() > 0){
            ui->search_edit->clear();
            searchClear->setIcon(QIcon(":/images/close_transparent.png"));
            ui->search_edit->clearFocus();
            // slot_side_chat();
        }
    });

    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_apply_friend_success, [this](std::shared_ptr<AddFriendApply> ai){
        if(ai == nullptr){
            return;
        }

        std::shared_ptr<ApplyInfo> _ai(new ApplyInfo(ai->_from_uid,
                                                     ai->_desc,
                                                     ai->_icon,
                                                     ai->_nick,
                                                     ai->_sex,0));
        auto& apply_list = UserMgr::GetInstance()->getApplyList();
        apply_list.push_back(_ai);
        ui->side_contact_lb->setUnReadCount(apply_list.size());
    });

    ui->search_edit->setLimitLength(20);
    ui->search_edit->installEventFilter(this);
    this->installEventFilter(this);

    ui->chat_data_wid->loadFriendApply(apply_list);
    if(!apply_list.empty()){
        ui->side_contact_lb->setUnReadCount(apply_list.size());
    }

    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_notify_friend_success, this, &ChatDialog::AddChatUser);
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_text_chat_msg, this, &ChatDialog::slot_text_chat_msg);
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_notify_text_chat_msg, this, &ChatDialog::slot_text_chat_msg);
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_load_chat_thread, this, &ChatDialog::slot_load_chat_thread);
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_load_chat_message, this, &ChatDialog::slot_load_chat_message);
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_auth_friend_success, this, &ChatDialog::slot_add_chat_item);
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_upload_img_chat_message, this, &ChatDialog::slot_upload_img_chat_message);
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_download_file, this, &ChatDialog::slot_download_file);
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_notify_download_file, this, &ChatDialog::slot_notify_download_file);
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_upload_video_chat_msg, this, &ChatDialog::slot_upload_video_chat_message);
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_download_video_chat_msg, this, &ChatDialog::slot_download_video_chat_msg);
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_close_tip_wid, [this](){
        _toast->hide();
    });
    connect(FileTcpMgr::GetInstance().get(), &FileTcpMgr::sig_reset_label_icon, this, &ChatDialog::slot_reset_label_icon);
    connect(FileTcpMgr::GetInstance().get(), &FileTcpMgr::sig_download_img_msg_chat_finished, this, &ChatDialog::slot_download_img_chat_data);
    connect(FileTcpMgr::GetInstance().get(), &FileTcpMgr::sig_download_video_msg_chat_finished, this, &ChatDialog::slot_download_video_chat_data);
    connect(ui->chat_data_wid, &ChatPage::sig_request_download_img, this,
            [this](qint64 msg_id, qint64 thread_id, QString name, int send_uid){
        slot_download_file(msg_id, thread_id, QString(), name, send_uid);
    });

    connect(ui->chat_data_wid, &ChatPage::sig_update_head_icon, this, &ChatDialog::InitSideHeadIcon);

    // addContactUserList();
    addChatUserList();
    InitSideHeadIcon();

    _timer = new QTimer(this);
    connect(_timer, &QTimer::timeout, [this](){
        auto uid = UserMgr::GetInstance()->getUid();
        QJsonObject json;
        json["uid"] = uid;
        QJsonDocument doc(json);
        QByteArray data = doc.toJson(QJsonDocument::Compact);
        qDebug() << "发送心跳检测数据包, uid: " << uid;
        emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_HEARTBEAT_REQ, data);
    });
    _timer->start(HEARTBEATTIME);
}

ChatDialog::~ChatDialog()
{
    _timer->stop();
    delete ui;
    if(_toast){
        delete _toast;
    }
}

void ChatDialog::setApplyList(const std::vector<std::shared_ptr<ApplyInfo> >& apply_list)
{
    if(!apply_list.empty()){
        UserMgr::GetInstance()->getApplyList() = apply_list;
    }
}

bool ChatDialog::eventFilter(QObject *o, QEvent *e)
{
    if(o == ui->search_edit && e->type() == QEvent::KeyPress){
        //slot_search();
    }

    if(o != ui->search_edit && e->type() == QMouseEvent::MouseButtonPress){
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(e);
        handleGlobalMousePress(mouseEvent);
    }
    return QDialog::eventFilter(o, e);
}

void ChatDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    static bool loaded = false;
    if (!loaded) {
        loaded = true;
        LoadChatThread();
    }
}

void ChatDialog::InitSideHeadIcon()
{
    QString head_icon = UserMgr::GetInstance()->getIcon();
    QRegularExpression regexp("^head_(\\d+)\\.jpg$");
    QRegularExpressionMatch match = regexp.match(head_icon);
    if(match.hasMatch()){
        QString appPath = QCoreApplication::applicationDirPath();
        QString iconPath = QDir::toNativeSeparators(appPath + QDir::separator() +
                                                    "static" + QDir::separator() + head_icon);
        QPixmap pix(iconPath);
        ui->side_head_lb->setPixmap(pix.scaled(ui->side_head_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->side_head_lb->setScaledContents(true);
    }else{
        QString standardDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir avatarsDir = standardDir + QDir::separator() + "avatars";

        if(avatarsDir.exists()){
            QString iconPath = avatarsDir.filePath(QFileInfo(head_icon).fileName());
            QPixmap pix(iconPath);
            if(!pix.isNull()){
                bool is_download = UserMgr::GetInstance()->IsDownLoad(head_icon);
                if(is_download){
                    // 使用默认头像
                    QString appPath = QCoreApplication::applicationDirPath();
                    QString iconPath = QDir::toNativeSeparators(appPath + QDir::separator() +
                                                                "static" + QDir::separator() + "WeChat.jpg");
                    QPixmap pix(iconPath);
                    ui->side_head_lb->setPixmap(pix.scaled(ui->side_head_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
                    ui->side_head_lb->setScaledContents(true);
                    return;
                }

                ui->side_head_lb->setPixmap(pix.scaled(ui->side_head_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
                ui->side_head_lb->setScaledContents(true);
            }else{
                qDebug() << "无法加载上传的头像: " << iconPath << ", 将使用默认头像";
                // 使用图标当头像
                QString appPath = QCoreApplication::applicationDirPath();
                QString iconPath = QDir::toNativeSeparators(appPath + QDir::separator() +
                                                            "static" + QDir::separator() + "WeChat.jpg");
                QPixmap pix(iconPath);
                ui->side_head_lb->setPixmap(pix.scaled(ui->side_head_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
                ui->side_head_lb->setScaledContents(true);

                QJsonObject json;
                json["name"] = head_icon;
                json["seq"] = 1;
                json["trans_size"] = 0;
                json["total_size"] = 0;
                QByteArray data = QJsonDocument(json).toJson(QJsonDocument::Compact);

                auto fileInfo = std::make_shared<FileInfo>();
                fileInfo->_name = head_icon;
                fileInfo->_seq = 1;
                fileInfo->_trans_size = 0;
                fileInfo->_total_size = 0;

                UserMgr::GetInstance()->AddDownLoadFile(head_icon, fileInfo);
                UserMgr::GetInstance()->AddResetLabel(head_icon, ui->side_head_lb);

                FileTcpMgr::GetInstance()->SendMsg(ID_DOWNLOAD_FILE_REQ, data);
            }

        }else{
            // 使用图标当头像
            QString appPath = QCoreApplication::applicationDirPath();
            QString iconPath = QDir::toNativeSeparators(appPath + QDir::separator() +
                                                        "static" + QDir::separator() + "WeChat.jpg");
            QPixmap pix(iconPath);
            ui->side_head_lb->setPixmap(pix.scaled(ui->side_head_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            ui->side_head_lb->setScaledContents(true);
        }
    }
}

void ChatDialog::slot_text_chat_msg(QVector<std::shared_ptr<ChatDataBase>> msg_vec, int recv_uid){
    if(msg_vec.isEmpty()){
        return;
    }

    // 获取对应的聊天用户项目
    QListWidgetItem* item = UserMgr::GetInstance()->getChatItemAdded()[recv_uid];
    QWidget* w = ui->chat_user_list->itemWidget(item);
    ChatUserItem* u_wid = qobject_cast<ChatUserItem*>(w);
    qDebug() << "friend thread: " << u_wid->GetThreadId();
    _unread_map[u_wid->GetThreadId()] += msg_vec.size();
    _unread_msg_count += _unread_map[u_wid->GetThreadId()];
    ui->side_chat_lb->setUnReadCount(_unread_msg_count);
    if(!u_wid){
        qDebug() << "u_wid无效";
    }else{
        u_wid->AddChatMessage(msg_vec);
        u_wid->setUnReadCount(_unread_map[u_wid->GetThreadId()]);

        ChatMsgType type = msg_vec.back()->GetMsgType();
        if(type == ChatMsgType::Text){
            u_wid->setLastMsg(msg_vec.back()->GetContent());
        }else if(type == ChatMsgType::Img){
            u_wid->setLastMsg("[图片]");
        }else if(type == ChatMsgType::File){
            u_wid->setLastMsg("[文件]");
        }else if(type == ChatMsgType::Video){
            u_wid->setLastMsg("[视频]");
        }
    }
}

void ChatDialog::slot_load_chat_thread()
{
    auto _threads = SqlMgr::GetInstance()->GetDiffChatThread();
    if(_threads.isEmpty()){
        qDebug() << "_threads is empty";
        _toast->hide();
        return;
    }

    for(auto& _thread : _threads){
        // 获取本地数据库中好友聊天信息列表
        auto thread_data = SqlMgr::GetInstance()->GetChatThreadData(_thread->_thread_id);
        SqlMgr::GetInstance()->GetChatThreadOtherUid(thread_data->GetOtherUid(), thread_data->GetThreadId());
        AddChatUser(thread_data);
    }

    _toast->hide();
}

void ChatDialog::slot_load_chat_message(QVector<std::shared_ptr<ChatDataBase>> msg_list)
{
    if(msg_list.isEmpty()){
        return;
    }

    auto self_uid = UserMgr::GetInstance()->getUid();
    auto& msg = msg_list.front();
    int other_uid = msg->GetSendUid();
    if(msg->GetSendUid() == self_uid){
        SqlMgr::GetInstance()->GetChatThreadOtherUid(other_uid, msg->GetThreadId());
    }
    QListWidgetItem* item = UserMgr::GetInstance()->getChatItemAdded()[other_uid];
    QWidget* w = ui->chat_user_list->itemWidget(item);
    ChatUserItem* u_wid = qobject_cast<ChatUserItem*>(w);
    if(!u_wid){
        return;
    }

    for(auto& data : msg_list){
        if(data && data->GetMsgType() == ChatMsgType::Img &&
            !std::dynamic_pointer_cast<ImgChatData>(data)){
            QString path = GetLocalFilePrefix(data->GetThreadId()) + data->GetContent();
            data = std::make_shared<ImgChatData>(data->GetMsgId(), data->GetThreadId(),
                                                  ChatMsgType::Img, data->GetContent(),
                                                  data->GetSendUid(), path);
        }
        auto img = std::dynamic_pointer_cast<ImgChatData>(data);
        if(img && (!QFile::exists(img->_total_path) || img->_pix.isNull())){
            qDebug() << "执行到这一步";
            slot_download_file(img->GetMsgId(), img->GetThreadId(), QString(),
                                img->GetContent(), img->GetSendUid());
        }
    }

    u_wid->AddChatMessage(msg_list);

    if(msg_list.back()->GetMsgType() == ChatMsgType::Img){
        u_wid->setLastMsg("[图片]");
    }else if(msg_list.back()->GetMsgType() == ChatMsgType::Video){
        u_wid->setLastMsg("[视频]");
    }else{
        u_wid->setLastMsg(msg_list.back()->GetContent());
    }

    if(ui->chat_data_wid->GetThreadId() == msg->GetThreadId()){
        ui->chat_data_wid->slot_load_chat_message(msg_list);
    }
}

void ChatDialog::slot_add_chat_item(std::shared_ptr<ChatThreadData> chat_data)
{
    if(chat_data == nullptr){
        return;
    }

    AddChatUser(chat_data);
}

void ChatDialog::slot_insert_chat_message(QVector<std::shared_ptr<ChatDataBase>> _data_list, qint64 _thread_id)
{
    int other_uid = 0;
    if(!SqlMgr::GetInstance()->GetChatThreadOtherUid(other_uid, _thread_id)){
        qDebug() << "获取好友id失败";
        return;
    }

    QListWidgetItem* item = UserMgr::GetInstance()->getChatItemAdded()[other_uid];
    auto* w = ui->chat_user_list->itemWidget(item);
    ChatUserItem* u_item = qobject_cast<ChatUserItem*>(w);
    u_item->AddChatMessage(_data_list);
}

void ChatDialog::slot_reset_label_icon(const QString &name)
{
    QLabel* label = UserMgr::GetInstance()->GetResetLabel(name);
    QString path_prefix = UserMgr::GetInstance()->GetPathPrefix() + "/avatars/";
    QString targetPath = path_prefix + name;
    qDebug() << "File path: " << targetPath;

    QPixmap pix(targetPath);
    label->setPixmap(pix.scaled(label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    label->setScaledContents(true);

    ui->chat_data_wid->ResetHeadIcon();
}

void ChatDialog::slot_upload_img_chat_message(std::shared_ptr<ImgChatData> img_msg)
{
    QFile file(img_msg->_total_path);
    if(!file.open(QIODevice::ReadOnly)){
        qDebug() << "Open the file failed, file name: " << img_msg->GetContent();
        return;
    }

    qint64 originalPos = file.pos();
    QCryptographicHash hash(QCryptographicHash::Md5);
    if(!hash.addData(&file)){
        qDebug() << "Open the file failed, file: " << img_msg->GetContent();
    }
    QString md5 = hash.result().toHex();

    file.seek(originalPos);

    auto fileInfo = std::make_shared<QFileInfo>(img_msg->_total_path);
    auto total_size = fileInfo->size();
    UserMgr::GetInstance()->AddFileInfoByMd5(md5, fileInfo);

    QByteArray read_data = file.read(MAX_FILE_READ_LEN);
    QString img_data = read_data.toBase64();
    file.close();

    QJsonObject json;
    json["md5"] = md5;
    json["name"] = img_msg->GetContent();
    json["seq"] = 1;
    json["trans_size"] = read_data.size();
    json["total_size"] = total_size;
    json["data"] = img_data;
    json["thread_id"] = img_msg->GetThreadId();
    json["message_id"] = img_msg->GetMsgId();
    json["uid"] = UserMgr::GetInstance()->getUid();
    int recv_uid = 0;
    SqlMgr::GetInstance()->GetChatThreadOtherUid(recv_uid, img_msg->GetThreadId());
    json["recv_uid"] = recv_uid;
    json["token"] = UserMgr::GetInstance()->getToken();
    json["finished"] = (read_data.size() > total_size);
    QByteArray send_data = QJsonDocument(json).toJson(QJsonDocument::Compact);
    FileTcpMgr::GetInstance()->SendMsg(ID_UPLOAD_IMG_CHAT_MSG_REQ, send_data);
}

void ChatDialog::slot_download_file(qint64 msg_id, qint64 thread_id, QString md5, QString name, int send_uid)
{
    qDebug() << "name: " << name;
    if(name.isEmpty()){
        return;
    }

    QString local_path = GetLocalFilePrefix(thread_id) + name;
    if(QFile::exists(local_path)){
        qDebug() << "Local chat image already exists: " << local_path;
        return;
    }

    if(UserMgr::GetInstance()->GetDownLoadFile(name) != nullptr){
        qDebug() << "Chat image is already downloading: " << name;
        return;
    }

    qint64 total_size = 0;
    auto file_info = UserMgr::GetInstance()->GetFileInfoByMd5(md5);
    if(file_info != nullptr){
        total_size = file_info->size();
    }

    auto fileInfo = std::make_shared<FileInfo>(name, 1, 0, total_size);
    QString dir_name = "thread_id_" + QString::number(thread_id);
    UserMgr::GetInstance()->AddDownLoadFile(name, fileInfo, dir_name);

    QJsonObject json;
    json["name"] = name;
    json["seq"] = 1;
    json["trans_size"] = fileInfo->_trans_size;
    json["total_size"] = fileInfo->_total_size;
    json["dir_name"] = dir_name;
    json["send_uid"] = send_uid;
    json["thread_id"] = thread_id;
    json["msg_id"] = msg_id;

    QByteArray send_data = QJsonDocument(json).toJson(QJsonDocument::Compact);
    FileTcpMgr::GetInstance()->SendMsg(ReqId::ID_DOWNLOAD_IMG_CHAT_MSG_REQ, send_data);
}

void ChatDialog::slot_download_img_chat_data(qint64 thread_id, std::shared_ptr<ChatDataBase> img_data)
{
    if(img_data == nullptr){
        qDebug() << "聊天图片数据为空";
        return;
    }

    int other_uid = 0;
    SqlMgr::GetInstance()->GetChatThreadOtherUid(other_uid, thread_id);
    QListWidgetItem* item = UserMgr::GetInstance()->getChatItemAdded()[other_uid];
    QWidget* w = ui->chat_user_list->itemWidget(item);
    ChatUserItem* u_wid = qobject_cast<ChatUserItem*>(w);
    if(u_wid){
        QVector<std::shared_ptr<ChatDataBase>> msg_list;
        msg_list.push_back(img_data);
        u_wid->AddChatMessage(msg_list);
        u_wid->setLastMsg("[图片]");
    }

    ui->chat_data_wid->upsertImgMessage(img_data);
    qDebug() << "插入消息";
}

void ChatDialog::slot_notify_download_file(int send_uid, qint64 thread_id, qint64 msg_id, QString name, QString md5){
    QListWidgetItem* item = UserMgr::GetInstance()->getChatItemAdded()[send_uid];
    auto* w = ui->chat_user_list->itemWidget(item);
    ChatUserItem* u_wid = qobject_cast<ChatUserItem*>(w);
    if(u_wid){
        QString path = GetLocalFilePrefix(thread_id) + name;
        auto img = std::make_shared<ImgChatData>(msg_id, thread_id, ChatMsgType::Img, name, send_uid, path);
        QVector<std::shared_ptr<ChatDataBase>> msg_list;
        msg_list.push_back(img);
        u_wid->AddChatMessage(msg_list);
        u_wid->setLastMsg("[图片]");
        ui->chat_data_wid->upsertImgMessage(img);
    }

    _unread_map[thread_id]++;

    if(u_wid){
        u_wid->setUnReadCount(_unread_map[thread_id]);
    }

    slot_download_file(msg_id, thread_id, md5, name, send_uid);
}

void ChatDialog::slot_upload_video_chat_message(std::shared_ptr<VideoChatData> videoData)
{
    // 先上传封面
    this->UploadVideoCover(videoData);

    // 上传本体
    this->UploadVideoBody(videoData);
}

void ChatDialog::slot_download_video_chat_msg(int send_uid, qint64 thread_id, qint64 msg_id, QString name, QString cover_name, int duration)
{
    // 先下载封面
    QString coverPath = GetDownLoadFilePrefix() + QDir::separator() + "video_covers" + QDir::separator() + cover_name;
    slot_download_video_cover(send_uid, thread_id, msg_id, cover_name);

    // 再下载本体
    QString videoPath = GetLocalFilePrefix(thread_id) + name;
    slot_download_video_body(send_uid, thread_id, msg_id, name);

    QListWidgetItem* item = UserMgr::GetInstance()->getChatItemAdded()[send_uid];
    auto* w = ui->chat_user_list->itemWidget(item);
    ChatUserItem* u_wid = qobject_cast<ChatUserItem*>(w);
    if(u_wid){
        QString path = GetLocalFilePrefix(thread_id) + name;
        auto video = std::make_shared<VideoChatData>(msg_id, thread_id, ChatMsgType::Video, name, send_uid, videoPath, coverPath, duration);
        QVector<std::shared_ptr<ChatDataBase>> msg_list;
        msg_list.push_back(video);
        u_wid->AddChatMessage(msg_list);
        u_wid->setLastMsg("[视频]");
        ui->chat_data_wid->upsertImgMessage(video);
    }
    _unread_map[thread_id]++;

    if(u_wid){
        u_wid->setUnReadCount(_unread_map[thread_id]);
    }
}

void ChatDialog::slot_download_video_cover(int send_uid, qint64 thread_id, qint64 msg_id, QString name)
{
    qDebug() << "name: " << name;
    if(name.isEmpty()){
        return;
    }

    QString local_path = GetDownLoadFilePrefix() + QDir::separator() + "video_covers" + QDir::separator() + name;
    if(QFile::exists(local_path)){
        qDebug() << "Local chat image already exists: " << local_path;
        return;
    }

    if(UserMgr::GetInstance()->GetDownLoadFile(name) != nullptr){
        qDebug() << "Chat image is already downloading: " << name;
        return;
    }

    auto fileInfo = std::make_shared<FileInfo>(name, 1, 0, 0);
    QString dir_name = "video_covers";
    UserMgr::GetInstance()->AddDownLoadFile(name, fileInfo, dir_name);

    QJsonObject json;
    json["name"] = name;
    json["seq"] = 1;
    json["trans_size"] = fileInfo->_trans_size;
    json["total_size"] = fileInfo->_total_size;
    json["dir_name"] = dir_name;
    json["send_uid"] = send_uid;
    json["thread_id"] = thread_id;
    json["msg_id"] = msg_id;

    QByteArray send_data = QJsonDocument(json).toJson(QJsonDocument::Compact);
    FileTcpMgr::GetInstance()->SendMsg(ReqId::ID_DOWNLOAD_VIDEO_COVER_CHAT_MSG_REQ, send_data);
}

void ChatDialog::slot_download_video_body(int send_uid, qint64 thread_id, qint64 msg_id, QString name)
{
    qDebug() << "name: " << name;
    if(name.isEmpty()){
        return;
    }

    QString local_path = GetLocalFilePrefix(thread_id) + name;
    if(QFile::exists(local_path)){
        qDebug() << "Local chat image already exists: " << local_path;
        return;
    }

    if(UserMgr::GetInstance()->GetDownLoadFile(name) != nullptr){
        qDebug() << "Chat image is already downloading: " << name;
        return;
    }

    auto fileInfo = std::make_shared<FileInfo>(name, 1, 0, 0);
    QString dir_name = "thread_id_" + QString::number(thread_id);
    UserMgr::GetInstance()->AddDownLoadFile(name, fileInfo, dir_name);

    QJsonObject json;
    json["name"] = name;
    json["seq"] = 1;
    json["trans_size"] = fileInfo->_trans_size;
    json["total_size"] = fileInfo->_total_size;
    json["dir_name"] = dir_name;
    json["send_uid"] = send_uid;
    json["thread_id"] = thread_id;
    json["msg_id"] = msg_id;

    QByteArray send_data = QJsonDocument(json).toJson(QJsonDocument::Compact);
    FileTcpMgr::GetInstance()->SendMsg(ReqId::ID_DOWNLOAD_VIDEO_BODY_CHAT_MSG_REQ, send_data);
}

void ChatDialog::slot_download_video_chat_data(qint64 thread_id, std::shared_ptr<VideoChatData> videoData)
{
    if(videoData == nullptr){
        qDebug() << "聊天图片数据为空";
        return;
    }

    int other_uid = 0;
    SqlMgr::GetInstance()->GetChatThreadOtherUid(other_uid, thread_id);
    QListWidgetItem* item = UserMgr::GetInstance()->getChatItemAdded()[other_uid];
    QWidget* w = ui->chat_user_list->itemWidget(item);
    ChatUserItem* u_wid = qobject_cast<ChatUserItem*>(w);
    if(u_wid){
        QVector<std::shared_ptr<ChatDataBase>> msg_list;
        msg_list.push_back(videoData);
        u_wid->AddChatMessage(msg_list);
        u_wid->setLastMsg("[视频]");
    }

    ui->chat_data_wid->upsertVideoMessage(videoData);
}

void ChatDialog::handleGlobalMousePress(QMouseEvent *e)
{
    if(_mode != ChatUIMode::SearchMode){
        return;
    }

    // 将鼠标点击的位置转化为搜索列表坐标系中的位置
    // 全局位置，x，y从显示屏幕左上角开始计算
    QPoint posConUserList = ui->search_list->mapFromGlobal(e->globalPos());
    QPoint posSideContact = ui->side_contact_lb->mapFromGlobal(e->globalPos());
    QPoint posSideChat = ui->side_chat_lb->mapFromGlobal(e->globalPos());
    if(!ui->search_list->rect().contains(posConUserList)
        && !ui->side_contact_lb->rect().contains(posSideContact)
        && !ui->side_chat_lb->rect().contains(posSideChat)){
        ui->search_edit->clear();
        //slot_side_chat();
        ShowSearch(false);
    }
}

void ChatDialog::ShowSearch(bool bsearch)
{
    if(bsearch){
        ui->chat_user_list->hide();
        ui->search_list->show();
        ui->con_user_list->hide();
        _mode = ChatUIMode::SearchMode;
    }else if(_state == ChatUIMode::ChatMode){
        ui->chat_user_list->show();
        ui->search_list->hide();
        ui->con_user_list->hide();
        _mode = ChatUIMode::ChatMode;
    }else if(_state == ChatUIMode::ContactMode){
        ui->chat_user_list->hide();
        ui->search_list->hide();
        ui->con_user_list->show();
        _mode = ChatUIMode::ContactMode;
    }
}

void ChatDialog::addChatUserList()
{
    auto thread_infos = SqlMgr::GetInstance()->GetChatThread();
    for(auto& info : thread_infos){
        auto thread_data = SqlMgr::GetInstance()->GetChatThreadData(info->_thread_id);
        if(!thread_data){
            qDebug() << "初始化chat user item 中chat data是空";
        }
        AddChatUser(thread_data);
        // QListWidgetItem* item = new QListWidgetItem;
        // ChatUserItem* u_wid = new ChatUserItem;
        // SqlMgr::GetInstance()->GetChatThreadOtherUid(thread_data->GetOtherUid(), thread_data->GetThreadId());
        // u_wid->setInfo(thread_data);
        // item->setSizeHint(u_wid->sizeHint());
        // ui->chat_user_list->addItem(item);
        // ui->chat_user_list->setItemWidget(item, u_wid);
        // UserMgr::GetInstance()->getChatItemAdded()[thread_data->GetOtherUid()] = item;
        // connect(u_wid, &ChatUserItem::sig_loading_data, ui->chat_data_wid, &ChatPage::slot_loading_data);
    }
}

void ChatDialog::addContactUserList()
{
    auto thread_infos = SqlMgr::GetInstance()->GetChatThread();
    for(auto& info : thread_infos){
        auto thread_data = SqlMgr::GetInstance()->GetChatThreadData(info->_thread_id);
        QListWidgetItem* item = new QListWidgetItem;
        ChatUserItem* u_wid = new ChatUserItem;
        u_wid->setInfo(thread_data);
        item->setSizeHint(u_wid->sizeHint());
        ui->chat_user_list->addItem(item);
        ui->chat_user_list->setItemWidget(item, u_wid);
        UserMgr::GetInstance()->getChatItemAdded()[thread_data->GetOtherUid()] = item;
        connect(u_wid, &ChatUserItem::sig_loading_data, ui->chat_data_wid, &ChatPage::slot_loading_contactUser);
    }
}

void ChatDialog::LoadChatThread()
{
    QJsonObject json;
    json["uid"] = UserMgr::GetInstance()->getUid();
    // 判断本地是否有数据库，有则从服务器拉取最新的聊天记录
    bool _b_exists = SqlMgr::GetInstance()->ExistsChatMessageTeble();
    if(_b_exists){
        qint64 _last_thread_id = SqlMgr::GetInstance()->GetLastThreadId();
        json["last_thread"] = _last_thread_id;
    }else{
        // 否则拉取全部聊天记录
        json["last_thread"] = 0;
    }

    QByteArray data = QJsonDocument(json).toJson(QJsonDocument::Compact);
    emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_LOAD_CHAT_THREAD_REQ, data);

    QPoint pos = mapToGlobal(QPoint(0,0));
    _toast = new Toast(this);
    _toast->showMessage("获取聊天列表中...");
}

void ChatDialog::AddChatUser(std::shared_ptr<ChatThreadData> chat_data)
{
    if (!chat_data) {
        qDebug() << "AddChatUser: chat_data is nullptr";
        return;
    }

    QListWidgetItem* item = new QListWidgetItem;
    ChatUserItem* u_wid = new ChatUserItem;
    SqlMgr::GetInstance()->GetChatThreadOtherUid(chat_data->GetOtherUid(), chat_data->GetThreadId());
    u_wid->setInfo(chat_data);
    u_wid->setUnReadCount(_unread_map[chat_data->GetThreadId()]);
    if(chat_data->GetChatData().size() > 0){
        auto msg_list = chat_data->GetChatData();
        auto last_msg = (--msg_list.end());
        if(last_msg->get()->GetMsgType() == ChatMsgType::Text){
            u_wid->setLastMsg(last_msg->get()->GetContent());
        }else if(last_msg->get()->GetMsgType() == ChatMsgType::Img){
            u_wid->setLastMsg("[图片]");
        }else if(last_msg->get()->GetMsgType() == ChatMsgType::File){
            u_wid->setLastMsg("[文件]");
        }else if(last_msg->get()->GetMsgType() == ChatMsgType::Video){
            u_wid->setLastMsg("[视频]");
        }
    }
    item->setSizeHint(u_wid->sizeHint());
    ui->chat_user_list->addItem(item);
    ui->chat_user_list->setItemWidget(item, u_wid);
    UserMgr::GetInstance()->getChatItemAdded()[chat_data->GetOtherUid()] = item;
    // connect(u_wid, &ChatUserItem::sig_loading_data, ui->chat_data_wid, &ChatPage::slot_loading_data);
    connect(u_wid, &ChatUserItem::sig_loading_data, [this, u_wid](std::shared_ptr<UserInfo> _ui, std::shared_ptr<ChatThreadData> _chat_data){
        _unread_msg_count -= _unread_map[_chat_data->GetThreadId()];
        _unread_map[_chat_data->GetThreadId()] = 0;
        ui->side_chat_lb->setUnReadCount(_unread_msg_count);
        u_wid->setUnReadCount(0);

        ui->chat_data_wid->slot_loading_data(_ui, _chat_data);
        QJsonObject json;
        json["thread_id"] = _chat_data->GetThreadId();
        json["last_message"] = _chat_data->GetLastMessageId();
        QByteArray data = QJsonDocument(json).toJson(QJsonDocument::Compact);
        emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_LOAD_CHAT_MESSAGE_REQ, data);
        _toast->showMessage("获取聊天记录中...");
    });
}

void ChatDialog::ChangeSideState()
{
    if(_mode == ChatUIMode::ChatMode){
        ui->side_chat_lb->setProperty("state", "press");
        ui->side_contact_lb->setProperty("state", "normal");
        ui->side_setting_lb->setProperty("state", "normal");
    }else if(_mode == ChatUIMode::ContactMode){
        ui->side_chat_lb->setProperty("state", "normal");
        ui->side_contact_lb->setProperty("state", "press");
        ui->side_setting_lb->setProperty("state", "normal");
    }else if(_mode == ChatUIMode::SettingsMode){
        ui->side_chat_lb->setProperty("state", "normal");
        ui->side_contact_lb->setProperty("state", "normal");
        ui->side_setting_lb->setProperty("state", "press");
    }

    repolish(ui->side_chat_lb);
    repolish(ui->side_contact_lb);
    repolish(ui->side_setting_lb);
}

void ChatDialog::UploadVideoCover(std::shared_ptr<VideoChatData> videoData)
{
    QFile file(videoData->_cover_path);
    if(!file.open(QIODevice::ReadOnly)){
        qDebug() << "Open the file failed, file name: " << videoData->GetContent();
        return;
    }

    qint64 originalPos = file.pos();
    QCryptographicHash hash(QCryptographicHash::Md5);
    if(!hash.addData(&file)){
        qDebug() << "Open the file failed, file: " << videoData->_cover_path;
    }
    QString md5 = hash.result().toHex();

    file.seek(originalPos);

    auto fileInfo = std::make_shared<QFileInfo>(videoData->_cover_path);
    auto total_size = fileInfo->size();
    QString coverName = fileInfo->fileName();
    UserMgr::GetInstance()->AddFileInfoByMd5(md5, fileInfo);

    QByteArray read_data = file.read(MAX_FILE_READ_LEN);
    QString img_data = read_data.toBase64();
    file.close();

    QJsonObject json;
    json["type"] = QString("cover");
    json["md5"] = md5;
    json["name"] = coverName;
    json["seq"] = 1;
    json["trans_size"] = read_data.size();
    json["total_size"] = total_size;
    json["data"] = img_data;
    json["thread_id"] = videoData->GetThreadId();
    json["message_id"] = videoData->GetMsgId();
    json["uid"] = UserMgr::GetInstance()->getUid();
    int recv_uid = 0;
    SqlMgr::GetInstance()->GetChatThreadOtherUid(recv_uid, videoData->GetThreadId());
    json["recv_uid"] = recv_uid;
    json["token"] = UserMgr::GetInstance()->getToken();
    json["finished"] = (read_data.size() > total_size);
    QByteArray send_data = QJsonDocument(json).toJson(QJsonDocument::Compact);
    FileTcpMgr::GetInstance()->SendMsg(ID_UPLOAD_VIDEO_CHAT_MSG_REQ, send_data);
}

void ChatDialog::UploadVideoBody(std::shared_ptr<VideoChatData> videoData)
{
    QFile file(videoData->_video_path);
    if(!file.open(QIODevice::ReadOnly)){
        qDebug() << "Open the file failed, file name: " << videoData->GetContent();
        return;
    }

    qint64 originalPos = file.pos();
    QCryptographicHash hash(QCryptographicHash::Md5);
    if(!hash.addData(&file)){
        qDebug() << "Open the file failed, file: " << videoData->GetContent();
    }
    QString md5 = hash.result().toHex();

    file.seek(originalPos);

    auto fileInfo = std::make_shared<QFileInfo>(videoData->_video_path);
    auto total_size = fileInfo->size();
    UserMgr::GetInstance()->AddFileInfoByMd5(md5, fileInfo);

    QByteArray read_data = file.read(MAX_FILE_READ_LEN);
    QString img_data = read_data.toBase64();
    file.close();

    QJsonObject json;
    json["type"] = QString("body");
    json["md5"] = md5;
    json["name"] = videoData->GetContent();
    json["seq"] = 1;
    json["trans_size"] = read_data.size();
    json["total_size"] = total_size;
    json["data"] = img_data;
    json["thread_id"] = videoData->GetThreadId();
    json["message_id"] = videoData->GetMsgId();
    json["uid"] = UserMgr::GetInstance()->getUid();
    json["duraion"] = videoData->_duration_sec;
    int recv_uid = 0;
    SqlMgr::GetInstance()->GetChatThreadOtherUid(recv_uid, videoData->GetThreadId());
    json["recv_uid"] = recv_uid;
    json["token"] = UserMgr::GetInstance()->getToken();
    json["finished"] = (read_data.size() > total_size);
    QByteArray send_data = QJsonDocument(json).toJson(QJsonDocument::Compact);
    FileTcpMgr::GetInstance()->SendMsg(ID_UPLOAD_VIDEO_CHAT_MSG_REQ, send_data);
}

// void ChatDialog::slot_side_chat()
// {
//     ui->side_chat_lb->setProperty("state", "press");
//     ui->side_contact_lb->setProperty("state", "normal");
//     repolish(ui->side_chat_lb);
//     repolish(ui->side_contact_lb);

//     ui->search_list->hide();
//     ui->con_user_list->hide();
//     ui->chat_user_list->show();
//     ui->chat_data_wid->setCurrentIndex(currentChatPageIndex);
//     _mode = ChatUIMode::ChatMode;
// }

// void ChatDialog::slot_search(){
//     ui->chat_user_list->hide();
//     ui->con_user_list->hide();
//     ui->search_list->show();
//     ui->search_list->setSearchEdit(ui->search_edit);
//     currentChatPageIndex = ui->chat_data_wid->currentIndex();
//     ui->chat_data_wid->setCurrentIndex(0);
//     _mode = ChatUIMode::SearchMode;
// }

// void ChatDialog::slot_side_con()
// {
//     ui->side_chat_lb->setProperty("state", "normal");
//     ui->side_contact_lb->setProperty("state", "press");
//     repolish(ui->side_chat_lb);
//     repolish(ui->side_contact_lb);

//     ui->chat_user_list->hide();
//     ui->search_list->hide();
//     ui->con_user_list->show();
//     _mode = ChatUIMode::ContactMode;
// }