#pragma once
#include "MysqlConPool.h"
#include <vector>
#include "data.h"

class MysqlDao{
public:
	MysqlDao();
	~MysqlDao();
	std::shared_ptr<UserInfo> GetUserInfo(const int& uid);
	bool UpdateUserHeadIcon(const int& uid, const std::string& icon_path);
private:
	std::unique_ptr<MysqlConPool> _pool;
};

