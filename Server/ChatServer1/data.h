#pragma once
#include <string>

struct UserInfo {
	UserInfo(): name(""), uid(0), email(""), pwd(""), desc(""), sex(0), icon(""), back(""), nick("")
	{}

	std::string name;
	int uid;
	std::string email;
	std::string pwd;
	std::string desc;
	int sex;
	std::string icon;
	std::string back;
	std::string nick;
};

struct ApplyInfo{
	ApplyInfo(): from_uid(0), name(""), desc(""), nick(""), icon(""), sex(0), status(0) {}

	int from_uid;
	std::string desc;
	std::string name;
	std::string nick;
	std::string icon;
	int sex;
	int status;
};

struct ChatThreadInfo {
	int64_t thread_id;
	std::string type;
	int user1_id;
	int user2_id;
	UserInfo info;
};

class ChatDataBase {
public:
	ChatDataBase(int64_t msg_id, int64_t thread_id, int msg_type,
		std::string content, int send_uid) : _msg_id(msg_id), _thread_id(thread_id),
		_msg_type(msg_type), _content(content), _send_uid(send_uid) {}
    int64_t GetMsgId() { return _msg_id; }
    int64_t GetThreadId() { return _thread_id; }
    int GetMsgType() { return _msg_type; }
    std::string GetContent() { return _content; }
    int GetSendUid() { return _send_uid; }
    std::string GetMsgContent() { return _content; }
	int64_t& GetMsgIdRef() { return _msg_id; }
private:
    //消息id
    int64_t _msg_id;
    //会话id
    int64_t _thread_id;
    //文本信息为1，图片为2，文件为3
    int _msg_type;
    std::string _content;
    //发送者id
    int _send_uid;
};