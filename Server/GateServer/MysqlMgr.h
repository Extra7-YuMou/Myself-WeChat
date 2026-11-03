#pragma once
#include "MysqlDao.h"
#include "Singleton.h"

class MysqlMgr : public Singleton<MysqlMgr>{
	friend class Singleton<MysqlMgr>;
public:
	~MysqlMgr() = default;
	int RegUser(const std::string& user, const std::string& email, const std::string& pwd);
	bool CheckEmail(const std::string& name, const std::string& email);
	bool ResetPwd(const std::string& name, const std::string& newPwd);
	bool CheckPwd(const std::string& name, const std::string& pwd, UserInfo& userInfo);
private:
	MysqlMgr() = default;
	MysqlDao _dao;
};

