#pragma once
#include "Singleton.h"
#include "MysqlDao.h"

class MysqlMgr : public Singleton<MysqlMgr>{
	friend class Singleton<MysqlMgr>;
public:
	~MysqlMgr() = default;
	std::shared_ptr<UserInfo> GetUserInfo(const int& uid);
	bool UpdateUserHeadIcon(const int& uid, const std::string& icon_path);
private:
	MysqlMgr() = default;
	MysqlDao _dao;
};

