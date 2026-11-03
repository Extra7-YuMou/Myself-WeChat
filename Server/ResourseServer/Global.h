#pragma once
#include <functional>
#define TCP_ID_LEN 2
#define TCP_BODY_LEN 4
#define TCP_TOTAL_LEN 6
#define MAX_SEND_QUE_SIZE 10000
#define MAX_BUFFER_LEN 1024 * 8
#define MAX_FILE_SIZE 1024 * 4
#define LOGIC_WORKER_COUNT 4
#define FILE_WORKER_COUNT 4
#define REDIS_FILEINFO_EXPRIED_TIME 3600
#define MAX_VALID_ID 10000
#define MAX_VALID_BODY 200000

#define USER_BASE_INFO "ubaseinfo_"
#define USERTOKENPREFIX  "utoken_"
#define FILEUPLOADPREFIX "file_upload_"
#define FILEDOWNLOADPREFIX "fil_download_"

enum MSGID {
	// 上传头像
	MSG_UPLOAD_HEAD_ICON_REQ = 1028,
	MSG_UPLOAD_HEAD_ICON_RSP = 1029,
	// 下载文件
	MSG_DOWNLOAD_FILE_REQ = 1030,
	MSG_DOWNLOAD_FILE_RSP = 1031,
	// 上传聊天图片
	MSG_UPLOAD_IMG_CHAT_MSG_REQ = 1035,
	MSG_UPLOAD_IMG_CHAT_MSG_RSP = 1036,
	// 下载聊天图片
	MSG_DOWNLOAD_IMG_CHAT_MSG_REQ = 1038,
	MSG_DOWNLOAD_IMG_CHAT_MSG_RSP = 1039,
	// 上传视频文件
	MSG_UPLOAD_VIDEO_CHAT_MSG_REQ = 1045,
	MSG_UPLOAD_VIDEO_CHAT_MSG_RSP = 1046,
	// 下载视频
	MSG_DOWNLOAD_VIDEO_COVER_CHAT_MSG_REQ = 1048,
	MSG_DOWNLOAD_VIDEO_COVER_CHAT_MSG_RSP = 1049,
	MSG_DOWNLOAD_VIDEO_BODY_CHAT_MSG_REQ = 1050,
	MSG_DOWNLOAD_VIDEO_BODY_CHAT_MSG_RSP = 1051,
};

enum ErrorCodes {
	SUCCESS = 0,
	ERR_JSON = 1,
	ERR_NETWORK = 2,
	ERR_RPCFialed = 3,
	VerifyExpired = 4,
	VerifyCodeErr = 5,
	UserExists = 6,
	EmailNotMatch = 7,
	ResetPwdFailed = 8,
	PasswdInvalid = 9,
	TokenInvalid = 10,
	UidInvalid = 11,
	ERR_REDIS = 12,
	FriendApplyInvalid = 13,
	RemoteServerInvalid = 14,
	AuthFriendApplyInvalid = 15,
	UpdateUserInfo = 16,
	CreatePrivateChatFail = 17,
	SaveChatMsgFail = 18,
	LoadChatMessageFailed = 19,
	InsertChatMessageFailed = 20,
	SaveFileInfoFailed = 21,
	FileNotExists = 22,
	FileReadPermissionFailed = 23,
	SeqNotMatch = 24,
	FileReadFailed = 25,
};

class Defer {
public:
	Defer(std::function<void()> func): _func(func){}
	~Defer() {
		_func();
	}
private:
	std::function<void()> _func;
};

enum FileTaskType {
	NoType = 0,
	UploadHeadIcon = 1,
	UploadImgChatMsg = 2,
	UploadVideoCover = 3,
	UploadVideoBody = 4,
};