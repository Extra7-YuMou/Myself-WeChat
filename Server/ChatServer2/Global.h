#pragma once
#include <boost/asio.hpp>
#include <iostream>
#include <queue>
#include <mutex>
#include <memory>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <thread>
#include <condition_variable>
#include <map>
#include <json/value.h>
#include <json/json.h>
#include <json/reader.h>
#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/exception.h>
#include "data.h"


#define HEAD_ID_LENGTH 2
#define HEAD_LEN_LENGTH 4
#define TOTAL_HEAD_LENGTH 6
#define MAX_LENGTH 1024 * 2
#define MAX_SEND_SIZE 10000

#define USERIPPREFIX  "uip_"
#define USERTOKENPREFIX  "utoken_"
#define IPCOUNTPREFIX  "ipcount_"
#define USER_BASE_INFO "ubaseinfo_"
#define LOGIN_COUNT  "logincount"
#define NAME_INFO  "nameinfo_"
#define UID_INFO "uidinfo_"
#define USERLOGINIP "userip"
#define LOCK_PREFIX "lock_"
#define USER_SESSION_PREFIX "usession_"
#define LOCK_TIME_OUT 10
#define ACQUIRE_TIME_OUT 5
#define CHAT_THREAD_PAGE_SIZE 10
#define CHAT_MESSAGE_PAGE_SIZE 1000

using tcp = boost::asio::ip::tcp;

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
};

enum MSG_IDS {
	// 用户登录
	MSG_CHAT_LOGIN = 1005,
	// 用户登录回包
	MSG_CHAT_LOGIN_RSP = 1006,
	// 搜索用户信息
	MSG_SEARCH_USER_REQ = 1007,
	// 搜索用户信息回包
	MSG_SEARCH_USER_RSP = 1008,
	// 添加好友申请
	MSG_APPLY_FRIEND_REQ = 1009,
	// 添加好友申请回包，用于通知客户对端添加好友申请
	MSG_APPLY_FRIEND_RSP = 1010,
	// 添加好友
	MSG_AUTH_FRIEND_REQ = 1011,
	// 添加好友回包
	MSG_AUTH_FRIEND_RSP = 1012,
	// 通知申请方好友通过回包
	MSG_NOTIFY_FRIEND_RSP = 1013,
	// 发送聊天消息
	MSG_TEXT_CHAT_MSG_REQ = 1014,
	// 发送聊天消息回包
	MSG_TEXT_CHAT_MSG_RSP = 1015,
	// 通知接收方接收消息
	MSG_NOTIFY_TEXT_CHAT_MSG_RSP = 1016,
	// 通知下线
	MSG_NOTIFY_OFF_LINE = 1017,
	// 定时心跳检测
	MSG_HEARTBEAT_REQ = 1018,
	// 定时心跳检测回包
	MSG_HEARTBEAT_RSP = 1019,
	// 用户更新信息
	MSG_UPDATE_USERINFO_REQ = 1020,
	// 用户更新信息回包
	MSG_UPDATE_USERINFO_RSP = 1021,
	// 获取聊天列表
	MSG_LOAD_CHAT_THREAD_REQ = 1022,
	// 获取聊天列表回包
	MSG_LOAD_CHAT_THREAD_RSP = 1023,
	// 添加私聊
	MSG_CREATE_PRIVATE_CHAT_REQ = 1024,
	// 添加私聊回包
	MSG_CREATE_PRIVATE_CHAT_RSP = 1025,
	// 获取聊天信息
	MSG_LOAD_CHAT_MESSAGE_REQ = 1026,
	// 获取聊天信息回包
	MSG_LOAD_CHAT_MESSAGE_RSP = 1027,
};

class Defer {
public:
	Defer(std::function<void()> func) : _func(func){}
	~Defer() {
		_func();
	}

	std::function<void()> _func;
};

bool GetBaseInfo(const std::string& key, std::shared_ptr<UserInfo> ui);
bool GetUserByName(const std::string& key, std::shared_ptr<UserInfo> ui);
bool GetUserByUid(const std::string& key, std::shared_ptr<UserInfo> ui);
std::string Gbk2utf8(const char*);
