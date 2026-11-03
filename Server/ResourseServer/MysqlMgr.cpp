#include "MysqlMgr.h"

std::shared_ptr<UserInfo> MysqlMgr::GetUserInfo(const int& uid) {
	return _dao.GetUserInfo(uid);
}

bool MysqlMgr::UpdateUserHeadIcon(const int& uid, const std::string& icon_path) {
	return _dao.UpdateUserHeadIcon(uid, icon_path);
}