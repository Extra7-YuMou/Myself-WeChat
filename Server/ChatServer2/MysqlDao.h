#pragma once
#include "MysqlConPool.h"
#include "data.h"
#include <vector>

class MysqlDao{
public:
	MysqlDao();
	~MysqlDao();
	std::shared_ptr<UserInfo> GetUser(int uid);
	std::shared_ptr<UserInfo> GetUserByName(const std::string& name);
	std::shared_ptr<UserInfo> GetUserByUid(const int& uid);
	bool AddFriendApply(const int& from, const int& to, const std::string& desc, const std::string& back, const std::string& icon);
	std::vector<std::shared_ptr<ApplyInfo>> GetFriendApplyList(const int& touid);
	bool AuthFriendApply(const int& self_uid, const int& friend_uid,int64_t& thread_id);
	bool AddFriend(const int& self_uid, const int& friend_uid);
	std::vector<std::shared_ptr<ApplyInfo>> GetFriendList(const int& uid);
	bool UpdateUserInfo(const int& uid, const std::string& nick, const std::string& desc);
	bool GetChatThreadInfo(int uid, int64_t lastId, int pageSize, std::vector<std::shared_ptr<ChatThreadInfo>>& threads, bool& loadMore, int64_t& nextLastId);
	bool AddPrivateChat(int user1_id, int user2_id, int64_t& thread_id);
	bool InsertPrivateChat(int64_t& message_id, const int64_t& thread_id, const int& user1_id, const int& user2_id, std::string content, const int& type);
	bool GetChatMessage(int64_t& lastId, const int64_t& thread_id, int pageSize,
		std::vector<std::shared_ptr<ChatDataBase>>& data_list, bool& loadMore);
private:
	std::unique_ptr<MysqlConPool> _pool;
};

