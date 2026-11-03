#include "chatpage.h"
#include "ui_chatpage.h"
#include "global.h"
#include "bubbleframe.h"
#include "chatitembase.h"
#include "textbubble.h"
#include "picturebubble.h"
#include "chatview.h"
#include <QTextBlock>
#include <QTemporaryFile>
#include "userdata.h"
#include "UserMgr.h"
#include "tcpmgr.h"
#include <QListWidgetItem>
#include <QDir>
#include <QCryptographicHash>
#include "FileTcpMgr.h"
#include <QFile>
#include "videoplayerdialog.h"

ChatPage::ChatPage(QWidget *parent)
    : QStackedWidget(parent)
    , ui(new Ui::ChatPage)
{
    ui->setupUi(this);
    ui->emo_lb->setState("normal", "hover", "press",
                        "normal", "hover", "press");
    ui->file_lb->setState("normal", "hover", "press",
                         "normal", "hover", "press");
    ui->send_btn->setProperty("state", "unvalid");
    repolish(ui->send_btn);
    ui->send_btn->setEnabled(false);
    ui->nick_lb->setContentsMargins(10, 0, 0, 0);

    connect(ui->data_edit, &QTextEdit::textChanged, [this](){
        if(ui->data_edit->document()->isEmpty()){
            ui->send_btn->setEnabled(false);
            ui->send_btn->setProperty("state", "unvalid");
        }else{
            ui->send_btn->setEnabled(true);
            ui->send_btn->setProperty("state", "valid");
        }
        repolish(ui->send_btn);
    });

    connect(ui->file_lb, &CustomizeFileLabel::sig_appendFile, this, &ChatPage::slot_appendFile);
    connect(ui->apply_friend_page, &ApplyFriendPage::sig_switch_default_page, [this](){
        this->setCurrentIndex(0);
    });
    connect(ui->data_list, &QListView::clicked, this, &ChatPage::on_message_click);
    //connect(ui->apply_friend_page, &ApplyFriendPage::sig_add_chat_item, this, &ChatPage::sig_add_chat_item);
    //connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_load_chat_message, this, &ChatPage::slot_load_chat_message);

    m_msgModel = new MessageModel(this);
    m_msgDelegate = new MessageDelegate(this);

    ui->data_list->setModel(m_msgModel);
    ui->data_list->setItemDelegate(m_msgDelegate);
    ui->data_list->setFrameShape(QAbstractItemView::NoFrame);
    ui->data_list->setSelectionMode(QAbstractItemView::NoSelection);
    ui->data_list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    ui->data_list->setResizeMode(QListView::Adjust);
    ui->data_list->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);

    ui->data_edit->setStyleSheet("border: 2px solid #E0E0E0;border-radius:5px;");
}

ChatPage::~ChatPage()
{
    delete ui;
}

void ChatPage::loadFriendApply(const std::vector<std::shared_ptr<ApplyInfo>>& apply_list)
{
    for(const auto& info : apply_list){
        ui->apply_friend_page->addNewApply(info);
    }

    this->setCurrentIndex(3);
}

void ChatPage::addTextChat(std::shared_ptr<ChatDataBase> data)
{

}

void ChatPage::ResetHeadIcon()
{
    m_msgModel->setHeadIcon();
}

qint64 ChatPage::GetThreadId() const
{
    return _chat_data ? _chat_data->GetThreadId() : 0;
}

std::shared_ptr<ChatDataBase> ChatPage::EnsureDisplayMsg(std::shared_ptr<ChatDataBase> data)
{
    if(!data){
        return nullptr;
    }
    if(data->GetMsgType() != ChatMsgType::Img){
        return data;
    }

    auto img = std::dynamic_pointer_cast<ImgChatData>(data);
    if(img){
        return img;
    }

    QString path = GetLocalFilePrefix(data->GetThreadId()) + data->GetContent();
    return std::make_shared<ImgChatData>(data->GetMsgId(), data->GetThreadId(),
                                         ChatMsgType::Img, data->GetContent(),
                                         data->GetSendUid(), path);
}

void ChatPage::RequestMissingImg(std::shared_ptr<ChatDataBase> data)
{
    auto img = std::dynamic_pointer_cast<ImgChatData>(data);
    if(!img){
        return;
    }
    if(QFile::exists(img->_total_path) && !img->_pix.isNull()){
        return;
    }
    emit sig_request_download_img(img->GetMsgId(), img->GetThreadId(),
                                    img->GetContent(), img->GetSendUid());
}

void ChatPage::upsertImgMessage(std::shared_ptr<ChatDataBase> img_data)
{
    if(!img_data || !_chat_data || _chat_data->GetThreadId() != img_data->GetThreadId()){
        return;
    }

    _chat_data->GetChatData()[img_data->GetMsgId()] = img_data;
    m_msgModel->replaceOrAppend(img_data);
    ui->data_list->scrollToBottom();
}

void ChatPage::upsertVideoMessage(std::shared_ptr<VideoChatData> videoData)
{
    if(!videoData || !_chat_data || _chat_data->GetThreadId() != videoData->GetThreadId()){
        return;
    }

    _chat_data->GetChatData()[videoData->GetMsgId()] = videoData;
    m_msgModel->replaceOrAppend(videoData);
    ui->data_list->scrollToBottom();
}

void ChatPage::slot_loading_data(std::shared_ptr<UserInfo> _ui, std::shared_ptr<ChatThreadData> _chat_data)
{
    if(_ui == nullptr || _chat_data == nullptr){
        return;
    }

    this->_chat_data = _chat_data;
    _user_info = _ui;
    ui->nick_lb->setText(_ui->_nick);
    setCurrentIndex(ChatDataWidType::ChatType);

    QVector<std::shared_ptr<ChatDataBase>> msg_list;
    for(auto msg : _chat_data->GetChatData()){
        // msg = EnsureDisplayMsg(msg);
        // RequestMissingImg(msg);
        // if(msg->GetMsgType() == ChatMsgType::Video){
        //     if(!msg->GetContent().isEmpty() && (!QFile::exists(msg->GetContent()) || QFileInfo(msg->GetContent()).size() <= 0)){
        //         qDebug() << "检测到视频资源没下载，将下载资源，资源: " << msg->GetContent();
        //         std::shared_ptr<FileInfo> fileInfo = std::make_shared<FileInfo>(msg->GetContent());
        //         UserMgr::GetInstance()->AddDownLoadFile(msg->GetContent(), fileInfo);

        //         QJsonObject json;
        //         json["thread_id"] = msg->GetThreadId();
        //         json["name"] = msg->GetContent();
        //         json["dir_name"] = "thread_id_" + QString::number(msg->GetThreadId());
        //         json["trans_size"] = 0;
        //         json["seq"] = 1;
        //         json["send_uid"] = msg->GetSendUid();
        //         QByteArray data = QJsonDocument(json).toJson(QJsonDocument::Compact);
        //         emit FileTcpMgr::GetInstance()->sig_send_msg(ReqId::ID_DOWNLOAD_VIDEO_BODY_CHAT_MSG_REQ, data);
        //     }
        // }
        msg_list.append(msg);
    }

    std::sort(msg_list.begin(), msg_list.end(), [](const std::shared_ptr<ChatDataBase>& a, const std::shared_ptr<ChatDataBase>& b){
        return a->GetMsgId() < b->GetMsgId();
    });

    m_msgModel->setMessages(msg_list);
    ui->data_list->scrollToBottom();
}

void ChatPage::slot_appendFile(const QString &url)
{
    ui->data_edit->insertFromUrl(url);
}

void ChatPage::slot_loading_contactUser(std::shared_ptr<UserInfo> _ui)
{
    if(_ui == nullptr){
        return;
    }

    this->setCurrentIndex(ChatDataWidType::ContactUserInfo);

    QString appPath = QCoreApplication::applicationDirPath();
    QString iconPath = QDir::toNativeSeparators(appPath + QDir::separator() +
                                                "static" + QDir::separator() + _ui->_icon);
    qDebug() << "iconPath: " << iconPath;
    QPixmap pix(iconPath);
    pix = pix.scaled(QSize(80, 80), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->contact_usericon_lb->setPixmap(pix);
    ui->contact_usericon_lb->setScaledContents(true);
    ui->contact_username_lb->setText(_ui->_name);
    ui->contact_nick_lb->setText("昵称: " + _ui->_nick);
    ui->contact_desc_lb->setText("简介: " + _ui->_desc);
    ui->contact_back_lb->setText("备注: " + _ui->_back);

    ui->chat_lb->setState("normal", "hover", "press", "select", "select_hover", "select_press");
    ui->voice_lb->setState("normal", "hover", "press", "select", "select_hover", "select_press");
    ui->video_lb->setState("normal", "hover", "press", "select", "select_hover", "select_press");
}

void ChatPage::on_send_btn_clicked()
{
    if(_user_info == nullptr){
        return;
    }

    auto mTextEdit = ui->data_edit;
    const QVector<MsgInfo> _msg_list = mTextEdit->getMsgList();
    auto friend_uid = _user_info->_uid;
    auto self_uid = UserMgr::GetInstance()->getUid();
    auto self_nick = UserMgr::GetInstance()->getNick();
    auto self_icon = UserMgr::GetInstance()->getIcon();
    ChatRole role = ChatRole::Self;

    QJsonObject textObj;
    QJsonArray text_array;
    int text_size;
    for(int i = 0; i < _msg_list.size(); i++){
        // 长度不合规就跳过
        if(_msg_list[i]._content.length() > 1024){
            continue;
        }

        auto info = _msg_list[i];
        ChatMsgType type = info._msgFlag;
        std::shared_ptr<ChatDataBase> send_msg;

        if(type == ChatMsgType::Text){
            // 生成唯一id
            QString uuid_str = QUuid::createUuid().toString();
            if(text_size + info._content.length() > 1024){
                textObj["friend_uid"] = friend_uid;
                textObj["self_uid"] = self_uid;
                textObj["text_array"] = text_array;

                QJsonDocument doc(textObj);
                QByteArray data = doc.toJson(QJsonDocument::Compact);
                text_size = 0;
                textObj = QJsonObject();
                text_array = QJsonArray();
                emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_TEXT_CHAT_MSG_REQ, data);
            }

            text_size += info._content.length();
            QJsonObject obj;
            QByteArray msg = info._content.toUtf8();
            obj["content"] = QString::fromUtf8(msg);
            obj["type"] = ChatMsgType::Text;
            text_array.append(obj);
            send_msg = std::make_shared<TextChatData>(0, _chat_data->GetThreadId(),
                                                      ChatMsgType::Text, info._content, self_uid);
            // 缓存到本地等待服务器回应
            //将bubble和uid绑定，以后可以等网络返回消息后设置是否送达
            // auto txt_msg = std::make_shared<TextChatData>(uuidString, obj["content"].toString(),
            //                                               user_info->_uid, _user_info->_uid);
            // emit sig_append_send_chat_msg(txt_msg);
        }else if(type == ChatMsgType::Img){
            if(!text_array.isEmpty()){
                textObj["friend_uid"] = friend_uid;
                textObj["self_uid"] = self_uid;
                textObj["text_array"] = text_array;
                textObj["thread_id"] = _chat_data->GetThreadId();
                QJsonDocument doc(textObj);
                QByteArray data = doc.toJson(QJsonDocument::Compact);
                emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_TEXT_CHAT_MSG_REQ, data);
            }

            QJsonObject obj;
            obj["firend_uid"] = friend_uid;
            obj["self_uid"] = self_uid;
            obj["thread_id"] = _chat_data->GetThreadId();
            obj["name"] = generateUniqueFileName(info._name);
            obj["path"] = info._total_path;
            obj["type"] = ChatMsgType::Img;
            send_msg = std::make_shared<ImgChatData>(0, _chat_data->GetThreadId(),
                                                      ChatMsgType::Img, info._content,
                                                     self_uid, info._total_path);
            QByteArray data = QJsonDocument(obj).toJson(QJsonDocument::Compact);
            emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_IMG_CHAT_MSG_REQ, data);
        }else if(type == ChatMsgType::Video){
            // 封面与 UUID 已在 MessageTextEdit::insertVideo 中生成，这里只复用
            if (info._cover_path.isEmpty() || !QFile::exists(info._cover_path)
                || info._name.isEmpty() || info._cover_name.isEmpty()) {
                qDebug() << "视频封面或文件名缺失:" << info._total_path
                         << "cover:" << info._cover_path
                         << "name:" << info._name
                         << "cover_name:" << info._cover_name;
                continue;
            }

            int duration = extractVideoDuration(info._total_path);

            QJsonObject json;
            json["friend_uid"] = friend_uid;
            json["self_uid"] = self_uid;
            json["thread_id"] = _chat_data->GetThreadId();
            json["name"] = info._name;
            json["cover_name"] = info._cover_name;
            json["path"] = info._total_path;
            json["cover_path"] = info._cover_path;
            json["type"] = ChatMsgType::Video;
            json["duration"] = duration;
            QByteArray data = QJsonDocument(json).toJson(QJsonDocument::Compact);
            emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_VIDEO_CHAT_MSG_REQ, data);

            send_msg = std::make_shared<VideoChatData>(0, _chat_data->GetThreadId(),
                                                       ChatMsgType::Video, info._name, self_uid,
                                                       info._total_path, info._cover_path, duration);
        }

        m_msgModel->appendMessage(send_msg);
        ui->data_list->scrollToBottom();
    }

    qDebug() << "text array: " << text_array;
    textObj["friend_uid"] = friend_uid;
    textObj["self_uid"] = self_uid;
    textObj["text_array"] = text_array;
    textObj["thread_id"] = _chat_data->GetThreadId();
    QJsonDocument doc(textObj);
    QByteArray data = doc.toJson(QJsonDocument::Compact);

    text_size = 0;
    textObj = QJsonObject();
    text_array = QJsonArray();
    emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_TEXT_CHAT_MSG_REQ, data);
}

void ChatPage::slot_switch_setting()
{
    this->setCurrentIndex(ChatDataWidType::Settings);

    QString headIconName = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/avatars/" +
                           "head_icon_" + QString::number(UserMgr::GetInstance()->getUid()) + ".jpg";
    QPixmap pix(headIconName);
    pix = pix.scaled(QSize(200, 200), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->upload_head_lb->setPixmap(pix);
    ui->setting_nick_edit->setText(UserMgr::GetInstance()->getNick());
    ui->setting_desc_edit->setText(UserMgr::GetInstance()->getDesc());
}

void ChatPage::on_upload_head_btn_clicked()
{
    QString url = QFileDialog::getOpenFileName(nullptr, "上传头像", "",
                                               "图片文件(*.jpg *.png *.jpeg)");
    if(url.isEmpty()) return;

    QPixmap pix = ImageCropperDialog::getCroppedImage(url, 600, 400, CropperShape::CIRCLE);
    if(pix.isNull()){
        return;
    }

    pix = pix.scaled(ui->upload_head_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->upload_head_lb->setPixmap(pix);
    ui->upload_head_lb->setScaledContents(true);

    QString storageDir = QStandardPaths::writableLocation(QStandardPaths::StandardLocation::AppDataLocation);

    QDir dir(storageDir);
    if(!dir.exists("avatars")){
        if(!dir.mkpath("avatars")){
            qDebug() << "创建" << storageDir << "文件夹失败";
            QMessageBox::warning(this, "创建文件夹失败", "创建资源文件夹时失败，请检查权限或磁盘空间");
            return;
        }
    }

    QString headIconName = "head_icon_" + QString::number(UserMgr::GetInstance()->getUid()) + "." +
                           QFileInfo(url).fileName().split(".")[1];
    QString targetPath = dir.filePath("avatars/" + headIconName);
    if(!pix.save(targetPath, "PNG")){
        qDebug() << "头像资源保存失败";
        QMessageBox::warning(this, "头像资源保存失败", "头像资源保存失败，请检查权限或磁盘控件");
        return;
    }else{
        qDebug() << "头像资源保存成功. path: " << targetPath;
        UserMgr::GetInstance()->setIcon(targetPath);
        emit sig_update_head_icon();
    }

    QFile file(url);
    if(!file.open(QIODevice::ReadOnly)){
        qDebug() << "文件打开失败";
        return;
    }

    qint64 originalPos = file.pos();
    QCryptographicHash hash(QCryptographicHash::Md5);
    if(!hash.addData(&file)){
        qDebug() << "Failed to read data from file:" << url;
        return;
    }
    QString md5 = hash.result().toHex();
    file.seek(originalPos);

    auto fileInfo = std::make_shared<QFileInfo>(url);
    UserMgr::GetInstance()->AddFileInfoByMd5(md5, fileInfo);

    QByteArray file_data = file.read(MAX_FILE_READ_LEN);
    QString base64Data = file_data.toBase64();

    QJsonObject json;
    json["uid"] = UserMgr::GetInstance()->getUid();
    json["token"] = UserMgr::GetInstance()->getToken();
    json["md5"] = md5;
    json["name"] = headIconName;
    json["seq"] = 1;
    json["trans_size"] = file_data.size();
    json["total_size"] = fileInfo->size();
    json["data"] = base64Data;

    if(file_data.size() >= fileInfo->size()){
        json["finished"] = true;
    }else{
        json["finished"] = false;
    }

    QByteArray send_msg = QJsonDocument(json).toJson(QJsonDocument::Compact);
    FileTcpMgr::GetInstance()->SendMsg(ReqId::ID_UPLOAD_HEAD_ICON_REQ, send_msg);

    m_msgModel->setHeadIcon();
    file.close();
}

void ChatPage::on_setting_submit_btn_clicked()
{
    if(ui->setting_nick_edit->text().isEmpty()){
        QMessageBox::information(this, "提示", "昵称不能为空");
        return;
    }

    if(ui->setting_desc_edit->text().isEmpty()){
        QMessageBox::information(this, "提示", "简介不能为空");
        return;
    }

    QJsonObject json;
    json["uid"] = UserMgr::GetInstance()->getUid();
    json["nick"] = ui->setting_nick_edit->text();
    json["desc"] = ui->setting_desc_edit->text();
    QByteArray data = QJsonDocument(json).toJson(QJsonDocument::Compact);
    emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_UPDATE_USERINFO_REQ, data);
}

void ChatPage::on_message_click(const QModelIndex &index)
{
    if(!index.isValid()){
        qDebug() << "当前选项为空";
    }

    auto _videoPath = index.data(MessageModel::RoleVideoPath).toString();
    if(_videoPath.isEmpty() || !QFile::exists(_videoPath) || QFileInfo(_videoPath).size() <= 0){
        qDebug() << "视频本体未下载，将重新下载" ;
        if(!_videoPath.isEmpty()){
            qDebug() << "视频路径： " << _videoPath;
        }
        // 发送下载视频请求

        auto send_uid = index.data(MessageModel::RoleSendUid).toInt();
        auto name = index.data(MessageModel::RoleContent).toString();
        auto thread_id = index.data(MessageModel::RoleThreadId).toLongLong();
        auto dir_name = "thread_id_" + QString::number(thread_id);

        auto fileInfo = std::make_shared<FileInfo>(name);
        UserMgr::GetInstance()->AddDownLoadFile(name, fileInfo, dir_name);

        QJsonObject json;
        json["name"] = name;
        json["seq"] = 1;
        json["trans_size"] = 0;
        json["dir_name"] = dir_name;
        json["thread_id"] = thread_id;
        json["send_uid"] = send_uid;
        QByteArray data = QJsonDocument(json).toJson(QJsonDocument::Compact);
        emit FileTcpMgr::GetInstance()->sig_send_msg(ReqId::ID_DOWNLOAD_VIDEO_BODY_CHAT_MSG_REQ, data);
        return;
    }

    auto* dlg = new VideoPlayerDialog(_videoPath);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->show();
}

void ChatPage::slot_load_chat_message(QVector<std::shared_ptr<ChatDataBase> > _data_list)
{
    if(_data_list.isEmpty()){
        return;
    }

    for(auto& data : _data_list){
        data = EnsureDisplayMsg(data);
        RequestMissingImg(data);
        m_msgModel->replaceOrAppend(data);
    }

    emit sig_insert_chat_message(_data_list, _data_list.begin()->get()->GetThreadId());
}

bool ChatPage::ExistsFile(qint64 tid, QString fileName)
{
    return QFile::exists(GetLocalFilePrefix(tid) + fileName);
}

bool ChatPage::ExistsVideoCover(QString videoName)
{
    videoName.replace(QRegularExpression("\\.[^.]+$"), "_cover.jpg");

    const QDir coverDir = QDir(GetDownLoadFilePrefix() + "video_covers");
    if(coverDir.exists(videoName)){
        return true;
    }
    return false;
}

