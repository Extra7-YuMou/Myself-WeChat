#include "MysqlMgr.h"

std::shared_ptr<UserInfo> MysqlMgr::GetUser(int uid) {
	return _dao.GetUser(uid);
}

std::shared_ptr<UserInfo> MysqlMgr::GetUserByName(const std::string& name) {
	return _dao.GetUserByName(name);
}

std::shared_ptr<UserInfo> MysqlMgr::GetUserByUid(const int& uid) {
	return _dao.GetUserByUid(uid);
}

bool MysqlMgr::AddFriendApply(const int& from, const int& to, const std::string& desc, const std::string& back, const std::string& icon) {
	return _dao.AddFriendApply(from, to, desc, back, icon);
}

std::vector<std::shared_ptr<ApplyInfo>> MysqlMgr::GetFriendApplyList(const int& touid) {
	return _dao.GetFriendApplyList(touid);
}

bool MysqlMgr::AuthFriendApply(const int& self_uid, const int& friend_uid, int64_t& thread_id) {
	return _dao.AuthFriendApply(self_uid, friend_uid, thread_id);
}

bool MysqlMgr::AddFriend(const int& self_uid, const int& friend_uid) {
	return _dao.AddFriend(self_uid, friend_uid);
}

std::vector<std::shared_ptr<ApplyInfo>> MysqlMgr::GetFriendList(const int& uid) {
	return _dao.GetFriendList(uid);
}

bool MysqlMgr::UpdateUserInfo(const int& uid, const std::string& nick, const std::string& desc) {
	return _dao.UpdateUserInfo(uid, nick, desc);
}

bool MysqlMgr::GetChatThreadInfo(int uid, int64_t lastId, int pageSize, 
	std::vector<std::shared_ptr<ChatThreadInfo>>& threads, bool& loadMore, int64_t& nextLastId) {
	return _dao.GetChatThreadInfo(uid, lastId, pageSize, threads, loadMore, nextLastId);
}

bool MysqlMgr::AddPrivateChat(int user1_id, int user2_id, int64_t& thread_id) {
	return _dao.AddPrivateChat(user1_id, user2_id, thread_id);
}

bool MysqlMgr::InsertPrivateChat(int64_t& message_id, const int64_t& thread_id, const int& user1_id, const int& user2_id, const std::string& content, const int& type) {
	return _dao.InsertPrivateChat(message_id, thread_id, user1_id, user2_id, content, type);
}

bool MysqlMgr::GetChatMessage(int64_t& lastId, const int64_t& thread_id, int pageSize,
	std::vector<std::shared_ptr<ChatDataBase>>& data_list, bool& loadMore) {
	return _dao.GetChatMessage(lastId, thread_id, pageSize, data_list, loadMore);
}

std::map<int64_t, int> MysqlMgr::GetUnReadMsgCount(const int& recv_uid, int64_t& unread_count) {
	return _dao.GetUnReadMsgCount(recv_uid, unread_count);
}

