#include "LogicSystem.h"
#include "StatusGrpcClient.h"
#include "MysqlMgr.h"
#include "RedisMgr.h"
#include "ConfigMgr.h"
#include "UserMgr.h"
#include "ChatGrpcClient.h"
#include "CServer.h"

LogicSystem::LogicSystem() : _b_stop(false) {
	_work_thread = std::thread(&LogicSystem::DealMsg, this);
	RegisterHandler();
}

LogicSystem::~LogicSystem(){
	_b_stop = true;
	_cond.notify_all();
	if (_work_thread.joinable()) {
		_work_thread.join();
	}
}

void LogicSystem::PostMsgToQue(std::shared_ptr<LogicNode> node){
	std::unique_lock<std::mutex> lock(_mtx);
	_msg_que.push(node);
	if (_msg_que.size()) {
		_cond.notify_one();
	}
}

void LogicSystem::handleLogin(std::shared_ptr<CSession> session, short message_id, std::string message_data){
	Json::Value root;
	Json::Reader reader;
	bool parse_result = reader.parse(message_data, root);
	if (!parse_result) {
		std::cout << "HandleLogin json 解析失败" << std::endl;
		root["error"] = ErrorCodes::ERR_JSON;
		std::string returnStr = root.toStyledString();
		session->Send(returnStr, message_id);
	}
	auto uid = root["uid"].asInt();
	auto token = root["token"].asString();
	std::cout << "user login uid is  " << uid << " user token  is "
		<< root["token"].asString() << std::endl;

	Json::Value rtvalue;
	Defer defer([this, session, &rtvalue]() {
		std::string returnStr = rtvalue.toStyledString();
		session->Send(returnStr, MSG_CHAT_LOGIN_RSP);
		});

	// 通过Redis验证
	std::string uid_str = std::to_string(uid);
	std::string token_key = USERTOKENPREFIX + uid_str;
	std::string token_value;
	bool b_token = RedisMgr::GetInstance()->Get(token_key, token_value);
	if (!b_token) {
		std::cout << "用户Uid: " << uid << "无效, 验证Token失败" << std::endl;
		rtvalue["error"] = ErrorCodes::UidInvalid;
	}

	if (token != token_value) {
		std::cout << "Token 验证失败" << std::endl;
		rtvalue["error"] = ErrorCodes::TokenInvalid;
	}

	auto self_server = ConfigMgr::GetInstance()->getSelfServerName();
	{
		auto identifier = RedisMgr::GetInstance()->acquireLock(uid_str, LOCK_TIME_OUT, ACQUIRE_TIME_OUT);
		Defer defer([&identifier, &uid_str]() {
			RedisMgr::GetInstance()->releaseLock(uid_str, identifier);
			});

		auto server_ip = RedisMgr::GetInstance()->HGet(USERLOGINIP, uid_str);
		if (!server_ip.empty()) {
			if (server_ip == self_server) {
				auto old_session = UserMgr::GetInstance()->getUidSession(uid);
				if (old_session) {
					old_session->notifyOffline(uid);
					old_session->getServer()->clearSession(old_session->getUuid());
				}
			}
			else {
				// 跨服踢人
				std::string server_ip = RedisMgr::GetInstance()->HGet(USERLOGINIP, uid_str);
				KickUserReq req;
				req.set_uid(uid);
				KickUserRsp rsp = ChatGrpcClient::GetInstance()->NotifyKickUser(server_ip, uid);
			}
		}
	}

	// 添加连接的服务器
	auto config = ConfigMgr::GetInstance();
	auto self_server_name = (*config)["SelfServer"]["Name"];
	bool b_insertip = RedisMgr::GetInstance()->HSet(USERLOGINIP, uid_str, self_server_name);
	if (!b_insertip) {
		std::cout << "插入用户ip地址失败" << std::endl;
		rtvalue["error"] = ErrorCodes::ERR_REDIS;
	}

	// 从GetBaseInfo函数获取数据
	std::shared_ptr<UserInfo> ui(new UserInfo);
	std::string baseInfo_key = USER_BASE_INFO + uid_str;
	bool b_getBaseInfo = GetBaseInfo(baseInfo_key, ui);
	if (!b_getBaseInfo) {
		std::cout << "uid无效" << std::endl;
		rtvalue["error"] = ErrorCodes::UidInvalid;
	}
	 
	// 增加服务器连接数
	std::string login_count = RedisMgr::GetInstance()->HGet(LOGIN_COUNT, (*config)["SelfServer"]["Name"]);
	if (login_count.empty()) {
		std::cout << "设置服务器连接数量失败" << std::endl;
		rtvalue["error"] = ErrorCodes::ERR_REDIS;
	}

	int new_login_count = std::atoi(login_count.c_str()) + 1;
	std::cout << "添加之后的连接数: " << new_login_count << std::endl;
	bool b_setConCount = RedisMgr::GetInstance()->HSet(LOGIN_COUNT, (*config)["SelfServer"]["Name"], std::to_string(new_login_count));
	if (!b_setConCount) {
		std::cout << "设置服务器连接数量失败" << std::endl;
		rtvalue["error"] = ErrorCodes::ERR_REDIS;
	}

	// 获取好友申请列表
	std::vector<std::shared_ptr<ApplyInfo>> apply_list = MysqlMgr::GetInstance()->GetFriendApplyList(uid);
	if (apply_list.empty()) {
		std::cout << "The user dont't has any friend apply" << std::endl;
	}
	else {
		std::cout << "发送好友申请列表信息" << std::endl;
		for (auto& info : apply_list) {
			Json::Value obj;
			obj["from_uid"] = info->from_uid;
			obj["desc"] = info->desc;
			obj["nick"] = info->nick;
			obj["sex"] = info->sex;
			obj["status"] = info->status;
			obj["icon"] = info->icon;
			rtvalue["apply_list"].append(obj);
		}
	}

	// 获取好友列表
	std::vector<std::shared_ptr<ApplyInfo>> friend_list;
	bool loadMore = true;
	int64_t nextLastId = 0;
	friend_list = MysqlMgr::GetInstance()->GetFriendList(uid);
	if (friend_list.empty()) {
		std::cout << "The user dont't has any friend" << std::endl;
	}
	else {
		for (auto& info : friend_list) {
			Json::Value obj;
			obj["uid"] = info->from_uid;
			obj["name"] = info->name;
			obj["nick"] = info->nick;
			obj["icon"] = info->icon;
			obj["sex"] = info->sex;
			rtvalue["friend_list"].append(obj);
		}
	}

	rtvalue["error"] = ErrorCodes::SUCCESS;
	rtvalue["uid"] = ui->uid;
	rtvalue["name"] = ui->name;
	rtvalue["email"] = ui->email;
	rtvalue["nick"] = ui->nick;
	rtvalue["desc"] = ui->desc;
	rtvalue["icon"] = ui->icon;
	rtvalue["sex"] = ui->sex;

	session->setUid(uid);
	UserMgr::GetInstance()->setUidSession(uid, session);
	RedisMgr::GetInstance()->Set(USER_SESSION_PREFIX + uid_str, session->getUuid());
}

void LogicSystem::handleLoadChatThread(std::shared_ptr<CSession> session, short id, std::string data){
	Json::Value root, rtvalue;
	Json::Reader reader;
	reader.parse(data, root);
	auto uid = root["uid"].asInt();
	auto last_thread_id = root["last_thread"].asInt64();
	std::cout << "uid: " << uid << ", last_thread_id: " << last_thread_id << std::endl;
	Defer defer([&rtvalue, session]() {
		session->Send(rtvalue.toStyledString(), MSG_LOAD_CHAT_THREAD_RSP);
		});

	std::vector<std::shared_ptr<ChatThreadInfo>> _threads;
	bool loadMore = false;
	int64_t nextLastId = 0;
	bool _b_get = MysqlMgr::GetInstance()->GetChatThreadInfo(uid, last_thread_id, CHAT_THREAD_PAGE_SIZE,
		_threads, loadMore, nextLastId);
	if (!_b_get) {
		std::cout << "获取聊天列表失败" << std::endl;
		rtvalue["error"] = ErrorCodes::UidInvalid;
		return;
	}

	rtvalue["error"] = ErrorCodes::SUCCESS;
	rtvalue["uid"] = uid;
	rtvalue["loadmore"] = loadMore;
	rtvalue["load_count"] = _threads.size();
	for (auto& info : _threads) {
		Json::Value obj;
		obj["thread_id"] = info->thread_id;
		obj["type"] = info->type;
		obj["user1_id"] = uid;
		obj["user2_id"] = info->info.uid;
		obj["other_uid"] = info->info.uid;
		obj["other_name"] = info->info.name;
		obj["other_nick"] = info->info.nick;
		obj["other_icon"] = info->info.icon;
		obj["other_sex"] = info->info.sex;
		rtvalue["chat_thread"].append(obj);
	}
}

void LogicSystem::handleAddPrivateChat(std::shared_ptr<CSession> session, short, std::string data){
	Json::Value root, rtvalue;
	Json::Reader reader;
	reader.parse(data, root);
	Defer defer([&rtvalue, session]() {
		session->Send(rtvalue.toStyledString(), MSG_CREATE_PRIVATE_CHAT_RSP);
		});

	auto user1_id = root["user1_id"].asInt();
	auto user2_id = root["user2_id"].asInt();

	int64_t thread_id = 0;
	bool res = MysqlMgr::GetInstance()->AddPrivateChat(user1_id, user2_id, thread_id);
	if (!res) {
		std::cout << "Create private chat failed" << std::endl;
		rtvalue["error"] = ErrorCodes::CreatePrivateChatFail;
		return;
	}

	rtvalue["thread_id"] = thread_id;
	rtvalue["user1_id"] = user1_id;
	rtvalue["user2_id"] = user2_id;
}

void LogicSystem::handleLoadChatMessage(std::shared_ptr<CSession> session, short id, std::string data){
	Json::Value root, rtvalue;
	Json::Reader reader;
	reader.parse(data, root);
	Defer defer([&rtvalue, session]() {
		session->Send(rtvalue.toStyledString(), MSG_LOAD_CHAT_MESSAGE_RSP);
		});

	auto thread_id = root["thread_id"].asInt64();
	auto last_message_id = root["last_message"].asInt64();
	bool loadMore = false;

	std::vector<std::shared_ptr<ChatDataBase>> _data_list;
	bool _b_getMessage = MysqlMgr::GetInstance()->GetChatMessage(last_message_id, thread_id, CHAT_MESSAGE_PAGE_SIZE,
		_data_list, loadMore);
	if (!_b_getMessage) {
		std::cout << "Get chat message failed" << std::endl;
		rtvalue["error"] = ErrorCodes::LoadChatMessageFailed;
		return;
	}

	rtvalue["error"] = ErrorCodes::SUCCESS;
	rtvalue["loadMore"] = loadMore;
	rtvalue["load_count"] = _data_list.size();

	for (auto& data : _data_list) {
		Json::Value tmp;
		tmp["message_id"] = data->GetMsgId();
		tmp["thread_id"] = data->GetThreadId();
		tmp["type"] = data->GetMsgType();
		tmp["content"] = data->GetContent();
		tmp["sender_id"] = data->GetSendUid();
		rtvalue["chat_message"].append(tmp);
	}
}

void LogicSystem::RegisterHandler(){
	_handlers[MSG_CHAT_LOGIN] = [this](std::shared_ptr<CSession> session, short id, std::string data) {
		handleLogin(session, id, data);
		};

	_handlers[MSG_SEARCH_USER_REQ] = [this](std::shared_ptr<CSession> session, short id, std::string data) {
		handleSearchUserByUid(session, id, data);
		};

	_handlers[MSG_APPLY_FRIEND_REQ] = [this](std::shared_ptr<CSession> session, short id, std::string data) {
		handleApplyFriend(session, id, data);
		};

	_handlers[MSG_AUTH_FRIEND_REQ] = [this](std::shared_ptr<CSession> session, short id, std::string data) {
		handleAuthFriend(session, id, data);
		};

	_handlers[MSG_TEXT_CHAT_MSG_REQ] = [this](std::shared_ptr<CSession> session, short id, std::string data) {
		handleDealChatTextMsg(session, id, data);
		};

	_handlers[MSG_HEARTBEAT_REQ] = [this](std::shared_ptr<CSession> session, short id, std::string data) {
		handleHeartBeat(session, id, data);
		};

	_handlers[MSG_UPDATE_USERINFO_REQ] = [this](std::shared_ptr<CSession> session, short id, std::string data) {
		handleUpdateUserInfo(session, id, data);
		};
	
	_handlers[MSG_LOAD_CHAT_THREAD_REQ] = [this](std::shared_ptr<CSession> session, short id, std::string data) {
		handleLoadChatThread(session, id, data);
		};

	_handlers[MSG_CREATE_PRIVATE_CHAT_REQ] = [this](std::shared_ptr<CSession> session, short id, std::string data) {
		handleAddPrivateChat(session, id, data);
		};

	_handlers[MSG_LOAD_CHAT_MESSAGE_REQ] = [this](std::shared_ptr<CSession> session, short id, std::string data) {
		handleLoadChatMessage(session, id, data);
		};
}

void LogicSystem::DealMsg(){
	for (;;) {
		std::unique_lock<std::mutex> lock(_mtx);
		_cond.wait(lock, [this]() {
			if (_b_stop) {
				return true;
			}
			return !_msg_que.empty();
			});

		if (_b_stop) {
			while (!_msg_que.empty()) {
				auto& node = _msg_que.front();
				auto iter_handler = _handlers.find(node->_recv_node->getId());
				if (iter_handler == _handlers.end()) {
					_msg_que.pop();
					continue;
				}
				iter_handler->second(node->_session, node->_recv_node->getId(),
					std::string(node->_recv_node->_msg, node->_recv_node->_total_len));
				_msg_que.pop();
			}
			break;
		}

		auto& node = _msg_que.front();
		auto iter_handler = _handlers.find(node->_recv_node->getId());
		if (iter_handler == _handlers.end()) {
			_msg_que.pop();
			continue;
		}
		iter_handler->second(node->_session, node->_recv_node->getId(),
			std::string(node->_recv_node->_msg, node->_recv_node->_total_len));
		_msg_que.pop();
	}
}

void LogicSystem::handleSearchUserByUid(std::shared_ptr<CSession> session, short id, std::string str){
	Json::Value root, rtvalue;
	Json::Reader reader;
	Defer defer([session, &rtvalue, id]() {
		session->Send(rtvalue.toStyledString(), MSG_SEARCH_USER_RSP);
		});

	bool b_parse_result = reader.parse(str, root);
	if (!b_parse_result) {
		std::cout << "Search UserInfo Json Parse failed" << std::endl;
		rtvalue["error"] = ErrorCodes::ERR_JSON;
		return;
	}

	std::shared_ptr<UserInfo> ui(new UserInfo);
	std::string uid_key = UID_INFO + root["uid"].asString();
	GetUserByUid(uid_key, ui);
	if (ui != nullptr && ui->uid != 0) {
		rtvalue["error"] = ErrorCodes::SUCCESS;
		rtvalue["uid"] = ui->uid;
		rtvalue["name"] = ui->name;
		rtvalue["email"] = ui->email;
		rtvalue["nick"] = ui->nick;
		rtvalue["icon"] = ui->icon;
		rtvalue["desc"] = ui->desc;
		rtvalue["sex"] = ui->sex;
	}
	else {
		std::cout << "Search UserInfo Uid invalid" << std::endl;
		rtvalue["error"] = ErrorCodes::UidInvalid;
	}
}

void LogicSystem::handleApplyFriend(std::shared_ptr<CSession> session, short id, std::string data){
	Json::Value root, rtvalue;
	Json::Reader reader;
	bool b_parse_result = reader.parse(data, root);
	if (!b_parse_result) {
		std::cout << "Add friend apply failed, json parse failed" << std::endl;
		rtvalue["error"] = ErrorCodes::ERR_JSON;
		session->Send(rtvalue.toStyledString(), MSG_APPLY_FRIEND_RSP);
		return;
	}

	int from_uid = root["from_uid"].asInt();
	std::cout << "self_uid: " << from_uid << std::endl;
	int to_uid = root["to_uid"].asInt();
	bool add_result = MysqlMgr::GetInstance()->AddFriendApply(from_uid, to_uid, root["desc"].asString(), root["back"].asString(), root["icon"].asString());
	if (!add_result) {
		std::cout << "Add friend apply failed" << std::endl;
		rtvalue["error"] = ErrorCodes::FriendApplyInvalid;
		session->Send(rtvalue.toStyledString(), MSG_APPLY_FRIEND_RSP);
		return;
	}

	// 通过redis获取对端服务器地址后通知对应服务器进行操作
	std::string to_ip = RedisMgr::GetInstance()->HGet(USERLOGINIP, root["to_uid"].asString());
	if (to_ip.empty()) {
		std::cout << "Get user login ip from redis failed" << std::endl;
		rtvalue["error"] = ErrorCodes::ERR_REDIS;
		session->Send(rtvalue.toStyledString(), MSG_APPLY_FRIEND_RSP);
		return;
	}

	// 如果客户端对端服务器和自己是同一个服务器
	if (to_ip == ConfigMgr::GetInstance()->getSelfServerName()) {
		bool b_addFriendApply = MysqlMgr::GetInstance()->AddFriendApply(from_uid, to_uid, root["desc"].asString(), root["back"].asString(), root["icon"].asString());
		if (!b_addFriendApply) {
			std::cout << "Add friend apply failed" << std::endl;
			rtvalue["error"] = ErrorCodes::FriendApplyInvalid;
			session->Send(rtvalue.toStyledString(), MSG_APPLY_FRIEND_RSP);
			return;
		}
		auto remote_session = UserMgr::GetInstance()->getUidSession(to_uid);
		if (!remote_session) {
			std::cout << "Remote server has not connect" << std::endl;
			rtvalue["error"] = ErrorCodes::RemoteServerInvalid;
			session->Send(rtvalue.toStyledString(), MSG_APPLY_FRIEND_RSP);
			return;
		}

		std::shared_ptr<UserInfo> si(new UserInfo);
		std::string uid_key = USER_BASE_INFO + std::to_string(from_uid);
		bool b_getBaseInfo = GetBaseInfo(uid_key, si);

		rtvalue["error"] = ErrorCodes::SUCCESS;
		rtvalue["from_uid"] = from_uid;
		rtvalue["to_uid"] = to_uid;
		rtvalue["desc"] = root["desc"].asString();
		rtvalue["back"] = root["back"].asString();
		rtvalue["icon"] = si->icon;
		rtvalue["nick"] = si->nick;
		rtvalue["sex"] = si->sex;
		remote_session->Send(rtvalue.toStyledString(), MSG_APPLY_FRIEND_RSP);
	}

	std::shared_ptr<UserInfo> si(new UserInfo);
	std::string uid_key = USER_BASE_INFO + std::to_string(from_uid);
	bool b_getBaseInfo = GetBaseInfo(uid_key, si);

	// 不同服务器
	AddFriendReq req;
	req.set_fromuid(from_uid);
	req.set_name(root["name"].asString());
	req.set_desc(root["desc"].asString());
	req.set_touid(to_uid);
	if (b_getBaseInfo) {
		req.set_icon(si->icon);
		req.set_nick(si->nick);
		req.set_sex(si->sex);
	}

	ChatGrpcClient::GetInstance()->NotifyAddFriend(to_ip, req);
}

void LogicSystem::handleAuthFriend(std::shared_ptr<CSession> session, short id, std::string data){
	Json::Value root, rtvalue;
	Json::Reader reader;
	bool b_parse_result = reader.parse(data, root);
	if (!b_parse_result) {
		std::cout << "Auth friend json parse failed" << std::endl;
		rtvalue["error"] = ErrorCodes::ERR_JSON;
		auto returnStr = rtvalue.toStyledString();
		session->Send(returnStr, MSG_AUTH_FRIEND_RSP);
	}

	Defer defer([session, &rtvalue]() {
		session->Send(rtvalue.toStyledString(), MSG_AUTH_FRIEND_RSP);
		});

	int self_uid = root["self_uid"].asInt();
	int friend_uid = root["friend_uid"].asInt();
	std::string chat = root["desc"].asString();
	int64_t thread_id = 0;
	int64_t message_id = 0;
	bool b_checkFriendApply = MysqlMgr::GetInstance()->AuthFriendApply(self_uid, friend_uid, thread_id);
	if (!b_checkFriendApply) {
		std::cout << "Auth friend apply failed" << std::endl;
		rtvalue["error"] = ErrorCodes::AuthFriendApplyInvalid;
		return;
	}
	bool b_addFriend = MysqlMgr::GetInstance()->AddFriend(self_uid, friend_uid);
	if (!b_addFriend) {
		std::cout << "Auth friend apply failed" << std::endl;
		rtvalue["error"] = ErrorCodes::UidInvalid;
		return;
	}
	std::cout << "thread_id: " << thread_id << std::endl;
	bool b_insert = MysqlMgr::GetInstance()->InsertPrivateChat(message_id, thread_id, friend_uid, self_uid, chat, 1);
	if (!b_insert) {
		std::cout << "Insert private chat failed" << std::endl;
		rtvalue["error"] = ErrorCodes::SaveChatMsgFail;
		return;
	}
	std::string defaultMsg = "我们已经是朋友了，快来一起聊天吧";
	defaultMsg = Gbk2utf8(defaultMsg.c_str());
	b_insert = MysqlMgr::GetInstance()->InsertPrivateChat(message_id, thread_id, self_uid, friend_uid, defaultMsg, 1);
	if (!b_insert) {
		std::cout << "Insert private chat failed" << std::endl;
		rtvalue["error"] = ErrorCodes::SaveChatMsgFail;
		return;
	}

	std::shared_ptr<UserInfo> ui(new UserInfo);
	std::string key = USER_BASE_INFO + std::to_string(friend_uid);
	GetBaseInfo(key, ui);
	std::cout << "friend_uid: " << friend_uid << ", ui uid: " << ui->uid << std::endl;

	rtvalue["error"] = ErrorCodes::SUCCESS;
	rtvalue["uid"] = friend_uid;
	rtvalue["name"] = ui->name;
	rtvalue["nick"] = ui->nick;
	rtvalue["icon"] = ui->icon;
	rtvalue["sex"] = ui->sex;
	rtvalue["message_id"] = message_id;
	rtvalue["thread_id"] = thread_id;
	rtvalue["friend_msg"] = chat;
	rtvalue["self_msg"] = defaultMsg;

	// 获取对方的服务器地址
	std::string friend_uid_str = std::to_string(friend_uid);
	std::string remote_server = RedisMgr::GetInstance()->HGet(USERLOGINIP, friend_uid_str);
	std::cout << "remote_server: " << remote_server << std::endl;
	std::cout << "self_server: " << ConfigMgr::GetInstance()->getSelfServerName() << std::endl;
	if (remote_server == ConfigMgr::GetInstance()->getSelfServerName()) {
		auto _session = UserMgr::GetInstance()->getUidSession(friend_uid);
		// 通知给申请方
		Json::Value notify;
		notify["error"] = ErrorCodes::SUCCESS;
		notify["friend_uid"] = self_uid;
		notify["self_uid"] = friend_uid;
		if (_session != nullptr) {
			std::shared_ptr<UserInfo> self_info(new UserInfo);
			std::string baseInfo_key = USER_BASE_INFO + std::to_string(self_uid);
			GetBaseInfo(baseInfo_key, self_info);
			if (self_info) {
				notify["name"] = self_info->name;
				notify["nick"] = self_info->nick;
				notify["icon"] = self_info->icon;
				notify["sex"] = self_info->sex;
				notify["thread_id"] = thread_id;
				notify["message_id"] = message_id;
				notify["self_msg"] = chat;
				notify["friend_msg"] = defaultMsg;
			}
			else {
				notify["error"] = ErrorCodes::UidInvalid;
			}
			std::cout << "notify: " << notify.toStyledString() << std::endl;
			_session->Send(notify.toStyledString(), MSG_NOTIFY_FRIEND_RSP);
		}

		return;
	}

	AuthFriendReq req;
	req.set_fromuid(self_uid);
	req.set_touid(friend_uid);
	auto* msgs = req.add_textmsgs();
	msgs->set_send_id(self_uid);
	msgs->set_thread_id(thread_id);
	msgs->set_message_id(message_id);
	msgs->set_msgcontent(chat);

	ChatGrpcClient::GetInstance()->NotifyAuthFriend(remote_server, req);
}

void LogicSystem::handleDealChatTextMsg(std::shared_ptr<CSession> session, short id, std::string data){
	Json::Value root, rtvalue;
	Json::Reader reader;
	bool b_parse_result = reader.parse(data, root);
	if (!b_parse_result) {
		std::cout << "DealTextMsg json prase failed" << std::endl;
		rtvalue["error"] = ErrorCodes::ERR_JSON;
		session->Send(rtvalue.toStyledString(), MSG_TEXT_CHAT_MSG_RSP);
		return;
	}

	int friend_uid = root["friend_uid"].asInt();
	// 自身就是发送方
	int self_uid = root["self_uid"].asInt();
	std::cout << "friend_uid: " << friend_uid << ", self_uid: " << self_uid << std::endl;

	Json::Value array = root["text_array"];
	rtvalue["error"] = ErrorCodes::SUCCESS;
	rtvalue["self_uid"] = self_uid;
	rtvalue["friend_uid"] = friend_uid;
	rtvalue["thread_id"] = root["thread_id"].asInt64();

	Defer defer([session, &rtvalue]() {
		session->Send(rtvalue.toStyledString(), MSG_TEXT_CHAT_MSG_RSP);
		});

	for (const auto& value : array) {
		auto content = value["content"].asString();
		auto type = value["type"].asInt();
		int64_t msgId = 0;
		bool _b_insertChat = MysqlMgr::GetInstance()->InsertPrivateChat(msgId, root["thread_id"].asInt64(), self_uid, friend_uid, content, type);
		if (!_b_insertChat) {
			std::cout << "插入消息失败" << std::endl;
			rtvalue["error"] = ErrorCodes::InsertChatMessageFailed;
			return;
		}
		Json::Value tmp;
		tmp["content"] = content;
		tmp["message_id"] = msgId;
		tmp["type"] = type;
		rtvalue["text_array"].append(tmp);
	}

	// 获取好友对端
	std::string self_server = ConfigMgr::GetInstance()->getSelfServerName();
	std::string remote_server = RedisMgr::GetInstance()->HGet(LOGIN_COUNT, self_server);
	if (!remote_server.empty()) {
		if (remote_server == self_server) {
			auto _session = UserMgr::GetInstance()->getUidSession(friend_uid);
			if (_session) {
				Json::Value notify;
				notify["thread_id"] = root["thread_id"].asInt64();
				notify["friend_uid"] = self_uid;
				notify["self_uid"] = friend_uid;
				for (const auto& msg : chat_vec) {
					Json::Value tmp;
					tmp["content"] = msg->GetContent();
					tmp["type"] = msg->GetMsgType();
					tmp["message_id"] = msg->GetMsgId();
					notify["text_array"].append(tmp);
				}
				_session->Send(notify.toStyledString(), MSG_NOTIFY_TEXT_CHAT_MSG_RSP);
			}
		}
		else {
			TextChatMsgReq req;
			req.set_fromuid(self_uid);
			req.set_touid(friend_uid);
			req.set_thread_id(root["thread_id"].asInt64());
			for (const auto& msg : chat_vec) {
				auto* chat_data = req.add_textmsgs();
				chat_data->set_msgcontent(msg->GetContent());
				chat_data->set_msgid(msg->GetMsgId());
				chat_data->set_type(msg->GetMsgType());
			}

			ChatGrpcClient::GetInstance()->NotifyTextChatMsg(remote_server, req, rtvalue);
		}
	}
}

void LogicSystem::handleHeartBeat(std::shared_ptr<CSession> session, short, std::string data){
	Json::Value rtvalue, root;
	Json::Reader reader;
	reader.parse(data, root);
	auto uid = root["uid"].asInt();

	rtvalue["error"] = ErrorCodes::SUCCESS;
	rtvalue["uid"] = uid;
	session->Send(rtvalue.toStyledString(), MSG_HEARTBEAT_RSP);
}

void LogicSystem::handleUpdateUserInfo(std::shared_ptr<CSession> session, short id, std::string data){
	Json::Value root, rtvalue;
	Json::Reader reader;
	reader.parse(data, root);
	Defer defer([session, &rtvalue]() {
		session->Send(rtvalue.toStyledString(), MSG_UPDATE_USERINFO_RSP);
		});

	auto uid = root["uid"].asInt();
	auto nick = root["nick"].asString();
	auto desc = root["desc"].asString();

	bool _b_update = MysqlMgr::GetInstance()->UpdateUserInfo(uid, nick, desc);
	if (!_b_update) {
		std::cout << "更新用户uid:" << uid << "信息失败" << std::endl;
		rtvalue["error"] = ErrorCodes::UpdateUserInfo;
		return;
	}

	// 更新redis，不需要分布式锁
	std::shared_ptr<UserInfo> ui(new UserInfo);
	ui = MysqlMgr::GetInstance()->GetUserByUid(uid);
	if (ui == nullptr) {
		std::cout << "更新用户uid:" << uid << "信息失败" << std::endl;
		rtvalue["error"] = ErrorCodes::UpdateUserInfo;
		return;
	}

	rtvalue["uid"] = uid;
	rtvalue["name"] = ui->name;
	rtvalue["email"] = ui->email;
	rtvalue["nick"] = ui->nick;
	rtvalue["icon"] = ui->icon;
	rtvalue["desc"] = ui->desc;
	rtvalue["sex"] = ui->sex;
	bool _b_setRedis = RedisMgr::GetInstance()->Set(USER_BASE_INFO + std::to_string(uid), rtvalue.toStyledString());
	if (!_b_setRedis) {
		std::cout << "更新用户uid: " << uid << "Redis数据失败" << std::endl;
		rtvalue["error"] = ErrorCodes::ERR_REDIS;
		return;
	}

	_b_setRedis = RedisMgr::GetInstance()->Set(UID_INFO + std::to_string(uid), rtvalue.toStyledString());
	if (!_b_setRedis) {
		std::cout << "更新用户uid: " << uid << "Redis数据失败" << std::endl;
		rtvalue["error"] = ErrorCodes::ERR_REDIS;
		return;
	}

	rtvalue["error"] = ErrorCodes::SUCCESS;
}

