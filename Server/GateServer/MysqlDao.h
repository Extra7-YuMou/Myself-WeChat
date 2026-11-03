#pragma once
#include "MysqlPool.h"

class MysqlDao{
public:
	MysqlDao();
	~MysqlDao();
	int RegUser(const std::string& name, const std::string& email, const std::string& pwd);
	bool CheckEmail(const std::string& name, const std::string& email);
	bool ResetPwd(const std::string& name, const std::string& newPwd);
	bool CheckPwd(const std::string& name, const std::string& pwd, UserInfo& userInfo);
private:
	std::unique_ptr<MysqlPool> _pool;
};

