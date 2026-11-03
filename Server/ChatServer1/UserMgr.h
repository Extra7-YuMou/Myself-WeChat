#pragma once
#include "Singleton.h"
#include <mutex>
#include <unordered_map>

class CSession;
class UserMgr : public Singleton<UserMgr>{
	friend class Singleton<UserMgr>;
public:
	~UserMgr();
	std::shared_ptr<CSession> getUidSession(int uid);
	void setUidSession(int uid, std::shared_ptr<CSession> session);
	void RmvUidSession(int uid);
private:
	UserMgr();

	std::mutex _session_mtx;
	std::unordered_map<int, std::shared_ptr<CSession>> _uid_to_session;
};

