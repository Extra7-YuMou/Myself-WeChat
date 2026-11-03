#include "MysqlMgr.h"

int MysqlMgr::RegUser(const std::string& user, const std::string& email, const std::string& pwd) {
	return _dao.RegUser(user, email, pwd);
}


bool MysqlMgr::CheckEmail(const std::string& name, const std::string& email) {
	return _dao.CheckEmail(name, email);
}

bool MysqlMgr::ResetPwd(const std::string& name, const std::string& newPwd) {
	return _dao.ResetPwd(name, newPwd);
}

bool MysqlMgr::CheckPwd(const std::string& name, const std::string& pwd, UserInfo& userInfo) {
	return _dao.CheckPwd(name, pwd, userInfo);
}