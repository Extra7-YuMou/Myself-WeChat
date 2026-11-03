#ifndef GLOBAL_H
#define GLOBAL_H
#define TCP_HEAD_LEN 6
#define HEARTBEATTIME 1000 * 30
#define MAX_FILE_READ_LEN 1024 * 4
#define MAX_VIDEO_READ_LEN 1024 * 64
#include <QWidget>
#include <functional>
#include <QStyle>
#include <iostream>
#include <memory>
#include <mutex>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTemporaryFile>
#include <QStandardPaths>
#include <QUuid>

using TCP_ID_LEN = quint16;
using TCP_BODY_LEN = quint32;

/**
 * @brief repolish 用于刷新组件样式
 */
extern std::function<void(QWidget*)> repolish;
extern std::function<QString(QString)> xorString;
extern QString gate_url_prefix;
extern const QString get_verify_success;
QString generateUniqueFileName(const QString &originalName);
QString GetDownLoadFilePrefix();
QString GetLocalFilePrefix(qint64 thread_id);
QString ffmpegPath();
QString extractVideoCover(const QString &videoPath, const QString &coverPath);
// 按指定封面路径抽帧；成功返回 coverOutputPath，失败返回空
QString makeCoverForVideo(const QString &videoPath, const QString &coverOutputPath);
QString ffprobePath();
int extractVideoDuration(const QString& videoPath);
QString formatDuration(int sec);

enum ReqId{
    // 获取验证码id
    ID_GET_VERIFY_CODE = 1001,
    // 注册用户id
    ID_REG_USER = 1002,
    // 更新密码id
    ID_RESET_PWD = 1003,
    // 登录id
    ID_USER_LOGIN = 1004,
    // 获取聊天服务器
    ID_CHAT_LOGIN = 1005,
    // 聊天服务器回包
    ID_CHAT_LOGIN_RSP = 1006,
    // 搜索用户信息
    ID_SEARCH_USER_REQ = 1007,
    // 搜索用户信息回包
    ID_SEARCH_USER_RSP = 1008,
    // 添加好友申请
    ID_APPLY_FRIEND_REQ = 1009,
    // 添加好友申请回包
    ID_APPLY_FRIEND_RSP = 1010,
    // 添加好友
    ID_AUTH_FRIEND_REQ = 1011,
    // 添加好友回包
    ID_AUTH_FRIEND_RSP = 1012,
    // 通知申请方好友申请通过
    ID_NOTIFY_FRIEND_RSP = 1013,
    // 发送消息
    ID_TEXT_CHAT_MSG_REQ = 1014,
    // 发送消息回包
    ID_TEXT_CHAT_MSG_RSP = 1015,
    // 通知接收接收消息
    ID_NOTIFY_TEXT_CHAT_MSG_RSP = 1016,
    // 通知下线
    ID_NOTIFY_OFF_LINE_RSP = 1017,
    // 定时心跳检测
    ID_HEARTBEAT_REQ = 1018,
    // 定时心跳检测回包
    ID_HEARTBEAT_RSP = 1019,
    // 用户更新信息
    ID_UPDATE_USERINFO_REQ = 1020,
    // 用户更新信息回包
    ID_UPDATE_USERINFO_RSP = 1021,
    // 获取聊天记录
    ID_LOAD_CHAT_THREAD_REQ = 1022,
    // 获取聊天记录回包
    ID_LOAD_CHAT_THREAD_RSP = 1023,
    // 获取指定聊天记录聊天消息
    ID_LOAD_CHAT_MESSAGE_REQ = 1026,
    // 获取指定聊天记录聊天消息回包
    ID_LOAD_CHAT_MESSAGE_RSP = 1027,
    // 上传头像
    ID_UPLOAD_HEAD_ICON_REQ = 1028,
    // 上传头像回包
    ID_UPLOAD_HEAD_ICON_RSP = 1029,
    // 下载文件
    ID_DOWNLOAD_FILE_REQ = 1030,
    ID_DOWNLOAD_FILE_RSP = 1031,
    // 发送聊天图片
    ID_IMG_CHAT_MSG_REQ = 1032,
    ID_IMG_CHAT_MSG_RSP = 1033,
    ID_UPLOAD_IMG_CHAT_MSG_REQ = 1035,
    ID_UPLOAD_IMG_CHAT_MSG_RSP = 1036,
    ID_NOTIFY_UPLOAD_IMG_CHAT_RSP = 1037,
    // 接收方下载图片到本地
    ID_DOWNLOAD_IMG_CHAT_MSG_REQ = 1038,
    ID_DOWNLOAD_IMG_CHAT_MSG_RSP = 1039,
    // 通知接收方准备下载聊天图片
    ID_NOTIFY_DOWNLOAD_IMG_CHAT_RSP = 1040,
    // 接收方下载聊天图片
    ID_DOWNLOAD_IMG_CHAT_REQ = 1041,
    ID_DOWNLOAD_IMG_CHAT_RSP = 1042,
    // 发送视频
    ID_VIDEO_CHAT_MSG_REQ = 1043,
    ID_VIDEO_CHAT_MSG_RSP = 1044,
    ID_UPLOAD_VIDEO_CHAT_MSG_REQ = 1045,
    ID_UPLOAD_VIDEO_CHAT_MSG_RSP = 1046,
    ID_NOTIFY_UPLOAD_VIDEO_CHAT_MSG_RSP = 1047,
    ID_DOWNLOAD_VIDEO_COVER_CHAT_MSG_REQ = 1048,
    ID_DOWNLOAD_VIDEO_COVER_CHAT_MSG_RSP = 1049,
    ID_DOWNLOAD_VIDEO_BODY_CHAT_MSG_REQ = 1050,
    ID_DOWNLOAD_VIDEO_BODY_CHAT_MSG_RSP = 1051,
    ID_NOTIFY_DOWNLOAD_VIDEO_CHAT_MSG_RSP = 1052,
};

enum ErrorCodes {
    SUCCESS = 0,
    ERR_JSON = 1,
    ERR_NETWORK = 2
};

enum Modules {
    // 注册用户模块
    REGISTERMOD = 0,
    // 重置模块
    RESETMOD = 1,
    // 登录模块
    LOGINMOD = 2,
};

enum TipErr{
    TIP_USER_ERR = 1,
    TIP_EMAIL_ERR = 2,
    TIP_PWD_ERR = 3,
    TIP_CONFIRM_ERR = 4,
    TIP_VERIFY_ERR = 5,
    TIP_PWD_CONFIRM = 6
};

enum ClickState{
    normal = 1,
    selected = 2
};

struct ServerInfo{
    int uid;
    QString host;
    QString port;
    QString token;
};

//聊天界面几种模式
enum ChatUIMode{
    SearchMode, //搜索模式
    ChatMode,   //聊天模式
    ContactMode, //联系模式
    SettingsMode, //设置模式
};

//自定义QListWidgetItem的几种类型
enum ListItemType{
    CHAT_USER_ITEM, //聊天用户
    CONTACT_USER_ITEM, //联系人用户
    SEARCH_USER_ITEM, //搜索到的用户
    ADD_USER_TIP_ITEM, //提示添加用户
    INVALID_ITEM,  //不可点击条目
    GROUP_TIP_ITEM, //分组提示条目
    LINE_ITEM,  //分割线
    APPLY_FRIEND_ITEM, //好友申请
};

enum ChatRole{
    Self,
    Other,
};

// 侧边栏类型
enum SideType{
    Chat,
    Contact,
    Setting,
};

enum ChatMsgType{
    Text = 1,
    Img = 2,
    File = 3,
    Video = 4,
};

struct MsgInfo{
    MsgInfo() = default;
    // 纯文本
    MsgInfo(ChatMsgType msgFlag, QString content): _msgFlag(msgFlag), _content(content){}
    // 文件资源(图片，视频，文件等)
    MsgInfo(ChatMsgType msgFlag, QPixmap pix, QString md5, QString name,
            int seq, int trans_size, qint64 total_size): _msgFlag(msgFlag),
        _md5(md5), _trans_size(trans_size), _total_size(total_size), _seq(seq){}

    ChatMsgType _msgFlag;    //"text,image,file,video"
    QString _content;    // 表示文件和图像的文件名,文本信息
    QPixmap _pixmap;     // 文件和图片的缩略图
    int _trans_size;
    qint64 _total_size;
    int _seq;
    QString _name;
    QString _md5;
    QString _total_path;    // 本地原文件绝对路径（视频/图片）
    QString _cover_path;    // 视频封面本地绝对路径
    QString _cover_name;    // 发给服务器的封面文件名（与 _name 同 UUID）
};

enum ChatDataWidType{
    Default = 0,
    Loading = 1,
    ChatType = 2,
    ApplyFriend = 3,
    ContactUserInfo = 4,
    Settings = 5,
};

enum MsgStatus{
    UNREAD,
    SENDFAIL,
    READED,
};

enum ChatFormType {
    Private = 1,
    Group = 2,
};

#endif // GLOBAL_H
