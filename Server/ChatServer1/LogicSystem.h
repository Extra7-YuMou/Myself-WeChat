#pragma once
#include "Global.h"
#include "Singleton.h"
#include "CSession.h"
#include "data.h"

class LogicSystem : public Singleton<LogicSystem>{
	using handler = std::function<void(std::shared_ptr<CSession>, short id, std::string str)>;
	friend class Singleton<LogicSystem>;
public:
	~LogicSystem();
	void PostMsgToQue(std::shared_ptr<LogicNode>);
private:
	void handleLogin(std::shared_ptr<CSession>, short id, std::string str);
	void handleSearchUserByUid(std::shared_ptr<CSession>, short id, std::string str);
	void handleApplyFriend(std::shared_ptr<CSession>, short, std::string);
	void handleAuthFriend(std::shared_ptr<CSession>, short, std::string);
	void handleDealChatTextMsg(std::shared_ptr<CSession>, short, std::string);
	void handleHeartBeat(std::shared_ptr<CSession>, short, std::string);
	void handleUpdateUserInfo(std::shared_ptr<CSession>, short, std::string);
	// 返回聊天会话列表
	void handleLoadChatThread(std::shared_ptr<CSession>, short, std::string);
	void handleAddPrivateChat(std::shared_ptr<CSession>, short, std::string);
	void handleLoadChatMessage(std::shared_ptr<CSession>, short, std::string);
	void handleDealChatImgMsg(std::shared_ptr<CSession>, short, std::string);
	void handleDealVideoChatMsg(std::shared_ptr<CSession>, short, std::string);
	void RegisterHandler();
	void DealMsg();
	LogicSystem();
	LogicSystem(const LogicSystem&) = delete;
	LogicSystem& operator=(const LogicSystem&) = delete;

	std::queue<std::shared_ptr<LogicNode>> _msg_que;
	std::mutex _mtx;
	std::condition_variable _cond;
	std::thread _work_thread;
	std::map<short, handler> _handlers;
	bool _b_stop;
	std::map<int, std::shared_ptr<UserInfo>> _users;
};

