#include "MysqlDao.h"
#include "ConfigMgr.h"


MysqlDao::MysqlDao(){
	auto config = ConfigMgr::GetInstance();
	std::string user = (*config)["Mysql"]["User"];
	std::string host = (*config)["Mysql"]["Host"];
	std::string port = (*config)["Mysql"]["Port"];
	std::string pwd = (*config)["Mysql"]["Passwd"];
	std::string schema = (*config)["Mysql"]["Schema"];
	_pool = std::make_unique<MysqlConPool>(5, user, pwd, host + ":" + port, schema);
}

MysqlDao::~MysqlDao() {
	_pool->Close();
}

std::shared_ptr<UserInfo> MysqlDao::GetUser(int uid){
	auto con = _pool->getConnection();
	std::shared_ptr<UserInfo> ui = nullptr;
	Defer defer([this, &con]() {
		_pool->returnConnection(std::move(con));
		});

	try {
		if (con == nullptr) {
			return ui;
		}

		std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("SELECT * FROM user WHERE uid = ?"));
		pstmt->setInt(1, uid);
		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		if (res->next()) {
			ui.reset(new UserInfo);
			ui->uid = res->getInt("uid");
			ui->email = res->getString("email");
			ui->name = res->getString("name");
			ui->pwd = res->getString("pwd");
			ui->desc = res->getString("desc");
			ui->icon = res->getString("icon");
			ui->sex = res->getInt("sex");
			ui->nick = res->getString("nick");
		}
	}
	catch (sql::SQLException& e) {
		std::cerr << "Excute sql command occure Error! " << std::endl;
		std::cerr << "Error Code: " << e.getErrorCode() << ", SQLState: "
			<< e.getSQLState() << std::endl;
		return ui;
	}
	return ui;
}

std::shared_ptr<UserInfo> MysqlDao::GetUserByName(const std::string& name){
	std::shared_ptr<UserInfo> ui;
	auto con = _pool->getConnection();
	Defer defer([this, &con]() {
		_pool->returnConnection(std::move(con));
		});

	try {
		if (con == nullptr) {
			return ui;
		}

		std::unique_ptr<sql::PreparedStatement> preStmt(con->_con->prepareStatement("SELECT * FROM user WHERE name = ?"));
		preStmt->setString(1, name);
		std::unique_ptr<sql::ResultSet> res(preStmt->executeQuery());
		if (res->next()) {
			ui.reset(new UserInfo);
			ui->uid = res->getInt("uid");
			ui->name = res->getString("name");
			ui->email = res->getString("email");
			ui->nick = res->getString("nick");
			ui->icon = res->getString("icon");
			ui->sex = res->getInt("sex");
			ui->desc = res->getString("desc");
		}
	}
	catch (sql::SQLException& e) {
		std::cerr << "Excute sql command occure Error! " << std::endl;
		std::cerr << "Error Code: " << e.getErrorCode() << ", SQLState: "
			<< e.getSQLState() << std::endl;
		return ui;
	}
	return ui;
}

std::shared_ptr<UserInfo> MysqlDao::GetUserByUid(const int& uid){
	std::shared_ptr<UserInfo> ui;
	auto con = _pool->getConnection();
	Defer defer([this, & con]() {
		_pool->returnConnection(std::move(con));
		});
	try {
		if (con == nullptr) {
			return ui;
		}

		std::unique_ptr<sql::PreparedStatement> preStmt(con->_con->prepareStatement("SELECT * FROM user WHERE uid = ?"));
		preStmt->setInt(1, uid);
		std::unique_ptr<sql::ResultSet> res(preStmt->executeQuery());
		if (res->next()) {
			ui.reset(new UserInfo);
			ui->uid = res->getInt("uid");
			ui->name = res->getString("name");
			ui->email = res->getString("email");
			ui->nick = res->getString("nick");
			ui->icon = res->getString("icon");
			ui->sex = res->getInt("sex");
			ui->desc = res->getString("desc");
		}
	}
	catch (sql::SQLException& e) {
		std::cerr << "Excute sql command occure Error! " << std::endl;
		std::cerr << "Error Code: " << e.getErrorCode() << ", SQLState: "
			<< e.getSQLState() << std::endl;
		return ui;
	}
	return ui;
}

bool MysqlDao::AddFriendApply(const int& from, const int& to, const std::string& desc, const std::string& back, const std::string& icon){
	auto con = _pool->getConnection();
	Defer defer([this, &con]() {
		_pool->returnConnection(std::move(con));
		});

	try {
		if (con == nullptr) {
			return false;
		}

		std::unique_ptr<sql::PreparedStatement> preStmt(con->_con->prepareStatement("INSERT INTO friend_apply (from_uid, to_uid, `desc`, back, icon) values (?,?,?,?,?) "
			"ON DUPLICATE KEY UPDATE from_uid = from_uid, to_uid = to_uid "));
		preStmt->setInt(1, from);
		preStmt->setInt(2, to);
		preStmt->setString(3, desc);
		preStmt->setString(4, back);
		preStmt->setString(5, icon);
		int row = preStmt->executeUpdate();
		if (row < 0) {
			return false;
		}
	}
	catch (sql::SQLException& e) {
		std::cerr << "Excute sql command occure Error! " << std::endl;
		std::cerr << "Error Code: " << e.getErrorCode() << ", SQLState: "
			<< e.getSQLState() << std::endl;
		return false;
	}
	return true;
}

std::vector<std::shared_ptr<ApplyInfo>> MysqlDao::GetFriendApplyList(const int& touid){
	auto con = _pool->getConnection();
	Defer defer([this, &con]() {
		_pool->returnConnection(std::move(con));
		});
	std::vector<std::shared_ptr<ApplyInfo>> _vec;
	try {
		if (con == nullptr) {
			return _vec;
		}

		std::unique_ptr<sql::PreparedStatement> preStmt(con->_con->prepareStatement(
			"SELECT u.uid, u.name, u.icon, u.nick, u.sex, fa.status, fa.desc FROM `friend_apply` fa, `user` u WHERE fa.to_uid = ? AND fa.from_uid = u.uid"));
		preStmt->setInt(1, touid);
		std::unique_ptr<sql::ResultSet> res(preStmt->executeQuery());
		while (res->next()) {
			ApplyInfo info;
			info.from_uid = res->getInt("uid");
			info.desc = res->getString("desc");
			info.nick = res->getString("nick");
			info.sex = res->getInt("sex");
			info.status = res->getInt("status");
			info.icon = res->getString("icon");
			info.name = res->getString("name");
			_vec.push_back(std::make_shared<ApplyInfo>(info));
		}
	}
	catch (sql::SQLException& e) {
		std::cerr << "Excute sql command occure Error! " << std::endl;
		std::cerr << "Error Code: " << e.getErrorCode() << ", SQLState: "
			<< e.getSQLState() << std::endl;
		return _vec;
	}

	return _vec;
}

bool MysqlDao::AuthFriendApply(const int& self_uid, const int& friend_uid, int64_t& thread_id) {
	auto con = _pool->getConnection();
	Defer defer([this, &con]() {
		_pool->returnConnection(std::move(con));
		});

	try {
		if (con == nullptr) {
			return false;
		}
		
		{
			std::unique_ptr<sql::PreparedStatement> preStmt(con->_con->prepareStatement("UPDATE `friend_apply` SET `status` = 1 WHERE `from_uid` = ? AND `to_uid` = ?"));
			preStmt->setInt(1, friend_uid);
			preStmt->setInt(2, self_uid);
			int row = preStmt->executeUpdate();
			if (row < 0) {
				return false;
			}
		}
		{
			int64_t last_thread_id = 0;
			std::unique_ptr<sql::PreparedStatement> preStmt(con->_con->prepareStatement("SELECT `id` FROM `chat_thread` ORDER BY `id` DESC LIMIT 1"));
			std::unique_ptr<sql::ResultSet> res(preStmt->executeQuery());
			if (res->next()) {
				last_thread_id = res->getInt64("id") + 1;
			}
			else {
				last_thread_id = 1;
			}
			thread_id = last_thread_id;
			if (!AddPrivateChat(self_uid, friend_uid, thread_id)) {
				return false;
			}
		}
	}
	catch (sql::SQLException& e) {
		std::cerr << "Excute sql command occure Error! " << std::endl;
		std::cerr << "Error Code: " << e.getErrorCode() << ", SQLState: "
			<< e.getSQLState() << std::endl;
		return true;
	}
	return true;
}


bool MysqlDao::AddFriend(const int& self_uid, const int& friend_uid){
	auto con = _pool->getConnection();
	Defer defer([this, &con]() {
		_pool->returnConnection(std::move(con));
		});

	try {
		if (con == nullptr) {
			return false;
		}

		const std::string queryStr = "INSERT INTO friend(self_id, friend_id) values(?,?)";
		std::unique_ptr<sql::PreparedStatement> preStmt(con->_con->prepareStatement(queryStr));
		preStmt->setInt(1, self_uid);
		preStmt->setInt(2, friend_uid);
		int row = preStmt->executeUpdate();
		if (row < 0) {
			return false;
		}

		const std::string insertStr = "INSERT INTO friend(self_id, friend_id) values(?, ?)";
		preStmt.reset(con->_con->prepareStatement(insertStr));
		preStmt->setInt(1, friend_uid);
		preStmt->setInt(2, self_uid);
		row = preStmt->executeUpdate();
		if (row < 0) {
			return false;
		}
	}
	catch (sql::SQLException& e) {
		std::cerr << "Excute sql command occure Error! " << std::endl;
		std::cerr << "Error Code: " << e.getErrorCode() << ", SQLState: "
			<< e.getSQLState() << std::endl;
		return false;
	}
	return true;
}

std::vector<std::shared_ptr<ApplyInfo>> MysqlDao::GetFriendList(const int& uid){
	auto con = _pool->getConnection();
	Defer defer([this, &con]() {
		_pool->returnConnection(std::move(con));
		});
	std::vector<std::shared_ptr<ApplyInfo>> _list;

	try {
		if (con == nullptr) {
			return _list;
		}

		std::unique_ptr<sql::PreparedStatement> preStmt(con->_con->prepareStatement(
			"SELECT u.*FROM `friend` f, `user` u WHERE f.self_id = ? AND f.friend_id = u.uid;"));
		preStmt->setInt(1, uid);
		std::unique_ptr<sql::ResultSet> res(preStmt->executeQuery());
		while (res->next()) {
			ApplyInfo info;
			info.from_uid = res->getInt("uid");
			info.desc = res->getString("desc");
			info.nick = res->getString("nick");
			info.sex = res->getInt("sex");
			info.icon = res->getString("icon");
			info.name = res->getString("name");
			_list.push_back(std::make_shared<ApplyInfo>(info));
		}
	}
	catch (sql::SQLException& e) {
		std::cerr << "Excute sql command occure Error! " << std::endl;
		std::cerr << "Error Code: " << e.getErrorCode() << ", SQLState: "
			<< e.getSQLState() << std::endl;
		return _list;
	}
	return _list;
}

bool MysqlDao::UpdateUserInfo(const int& uid, const std::string& nick, const std::string& desc){
	auto con = _pool->getConnection();
	Defer defer([&con, this]() {
		_pool->returnConnection(std::move(con));
		});

	try {
		if (con == nullptr) {
			return false;
		}

		std::unique_ptr<sql::PreparedStatement> preStmt(con->_con->prepareStatement("UPDATE `user` SET `nick` = ?, `desc` = ? WHERE `uid` = ?"));
		preStmt->setString(1, nick);
		preStmt->setString(2, desc);
		preStmt->setInt(3, uid);
		int row = preStmt->executeUpdate();
		if (row < 0) {
			return false;
		}
	}
	catch (sql::SQLException& e) {
		std::cerr << "Excute sql command occure Error! " << std::endl;
		std::cerr << "Error Code: " << e.getErrorCode() << ", SQLState: "
			<< e.getSQLState() << std::endl;
		return false;
	}
	return true;
}

bool MysqlDao::GetChatThreadInfo(int uid, int64_t lastId, int pageSize, std::vector<std::shared_ptr<ChatThreadInfo>>& threads, bool& loadMore, int64_t& nextLastId){
	auto con = _pool->getConnection();
	Defer defer([&con, this]() {
		_pool->returnConnection(std::move(con));
		});

	loadMore = false;
	nextLastId = lastId;

	try {
		if (con == nullptr) {
			return false;
		}

		// Ŀǰֻ������
		std::string queryStr =
			"SELECT "
			"ct.id AS thread_id, "
			"ct.type AS chat_type, "
			"u.uid AS other_uid, "
			"u.name AS other_name, "
			"u.nick AS other_nick, "
			"u.icon AS other_icon, "
			"u.sex AS other_sex "
			"FROM chat_thread ct "
			"JOIN private_chat pc ON ct.id = pc.thread_id AND pc.thread_id > ? "
			"JOIN user u ON(u.uid = pc.user1_id OR u.uid = pc.user2_id) "
			"JOIN user me ON me.uid = ? "
			"WHERE(pc.user1_id = ? OR pc.user2_id = ?) "
			"AND u.uid != ? "
			"LIMIT ?";
		std::unique_ptr<sql::PreparedStatement> preStmt(con->_con->prepareStatement(queryStr));
		preStmt->setInt64(1, lastId);
		preStmt->setInt(2, uid);
		preStmt->setInt(3, uid);
		preStmt->setInt(4, uid);
		preStmt->setInt(5, uid);
		preStmt->setInt(6, pageSize + 1);

		std::unique_ptr<sql::ResultSet> res(preStmt->executeQuery());
		std::vector<std::shared_ptr<ChatThreadInfo>> tmp;
		while (res->next()) {
			auto info = std::make_shared<ChatThreadInfo>();
			info->thread_id = res->getInt64("thread_id");
			info->type = res->getString("chat_type");
			info->info.uid = res->getInt("other_uid");
			info->info.name = res->getString("other_name");
			info->info.nick = res->getString("other_nick");
			info->info.icon = res->getString("other_icon");
			info->info.sex = res->getInt("other_sex");
			tmp.push_back(info);
		}

		if (tmp.size() > pageSize) {
			loadMore = true;
			tmp.pop_back();
		}

		if (!tmp.empty()) {
			nextLastId = tmp.back()->thread_id;
		}
		else {
			return true;
		}

		threads = std::move(tmp);
	}
	catch (sql::SQLException& e) {
		std::cerr << "Excute sql command occure Error! " << std::endl;
		std::cerr << "Error Code: " << e.getErrorCode() << ", SQLState: "
			<< e.getSQLState() << std::endl;
		return false;
	}
	return true;
}

bool MysqlDao::AddPrivateChat(int user1_id, int user2_id, int64_t& thread_id){
	auto con = _pool->getConnection();
	Defer defer([&con, this]() {
		_pool->returnConnection(std::move(con));
		});

	try {
		if (con == nullptr) {
			return false;
		}

		std::string check_thread_id = "SELECT `thread_id` FROM `private_chat` WHERE `user1_id` = ? AND `user2_id` = ? FOR UPDATE;";
		std::unique_ptr<sql::PreparedStatement> preStmt(con->_con->prepareStatement(check_thread_id));
		preStmt->setInt(1, user1_id);
		preStmt->setInt(2, user2_id);
		std::unique_ptr<sql::ResultSet> checkRes(preStmt->executeQuery());
		if (checkRes->next()) {
			thread_id = checkRes->getInt64("thread_id");
			con->_con->commit();
			return true;
		}

		std::string insert_chat_thread = "INSERT INTO `chat_thread`(`id`, `type`) values(?, 'private')";
		std::unique_ptr<sql::PreparedStatement> preStmt2(con->_con->prepareStatement(insert_chat_thread));
		preStmt2->setInt64(1, thread_id);
		int row = preStmt2->executeUpdate();
		if (row < 0) {
			thread_id = 0;
			con->_con->commit();
			return false;
		}

		std::string select_last_thread = "SELECT `thread_id` FROM `private_chat` ORDER BY `thread_id` DESC LIMIT 1;";
		std::unique_ptr<sql::PreparedStatement> preStmt3(con->_con->prepareStatement(select_last_thread));
		std::unique_ptr<sql::ResultSet> selectRes(preStmt3->executeQuery());
		if (!selectRes->next()) {
			thread_id = 0;
			con->_con->commit();
			return false;
		}
		thread_id = selectRes->getInt64("thread_id") + 1;

		std::string insert_private_chat = "INSERT INTO `private_chat`(`thread_id`, `user1_id`, `user2_id`) "
											"VALUES(?,?,?);";
		std::unique_ptr < sql::PreparedStatement> preStmt4(con->_con->prepareStatement(insert_private_chat));
		preStmt4->setInt64(1, thread_id);
		preStmt4->setInt(2, (user1_id < user2_id) ? user1_id : user2_id);
		preStmt4->setInt(3, (user1_id > user2_id) ? user1_id : user2_id);
		int rowInsert = preStmt4->executeUpdate();
		if (rowInsert < 0) {
			thread_id = 0;
			con->_con->commit();
			return false;
		}

		con->_con->commit();
	}
	catch (sql::SQLException& e) {
		std::cerr << "Excute sql command occure Error! " << std::endl;
		std::cerr << "Error Code: " << e.getErrorCode() << ", SQLState: "
			<< e.getSQLState() << std::endl;
		con->_con->rollback();
		return false;
	}
	return true;
}

bool MysqlDao::InsertPrivateChat(int64_t& message_id, const int64_t& thread_id, const int& user1_id, const int& user2_id, std::string content, const int& type){
	auto con = _pool->getConnection();
	Defer defer([&con, this]() {
		_pool->returnConnection(std::move(con));
		});

	try {
		if (con == nullptr) {
			return false;
		}

		{
			std::unique_ptr<sql::PreparedStatement> preStmt(con->_con->prepareStatement("SELECT * FROM `chat_thread` WHERE `id` = ?"));
			preStmt->setInt64(1, thread_id);
			std::unique_ptr<sql::ResultSet> res(preStmt->executeQuery());
			if (!res->next()) {
				return false;
			}
		}
		int64_t last_message_id = 0;
		{
			std::unique_ptr<sql::PreparedStatement> preStmt(con->_con->prepareStatement("SELECT `message_id` FROM `chat_message` ORDER BY `message_id` DESC LIMIT 1"));
			std::unique_ptr<sql::ResultSet> res(preStmt->executeQuery());
			if (res->next()) {
				last_message_id = res->getInt64("message_id") + 1;
			}
			else {
				last_message_id = 1;
			}
			message_id = last_message_id;
		}
		{
			std::cout << "thread id: " << thread_id << std::endl;
			std::cout << "last message id: " << last_message_id << std::endl;
			std::string queryStr = "INSERT INTO `chat_message`(`message_id`, `thread_id`, `sender_id`, `recv_id`, `type`, `content`, `status`) "
				"VALUES(?,?,?,?,?,?,0)";
			std::unique_ptr<sql::PreparedStatement> preStmt(con->_con->prepareStatement(queryStr));
			preStmt->setInt64(1, last_message_id);
			preStmt->setInt64(2, thread_id);
			preStmt->setInt(3, user1_id);
			preStmt->setInt(4, user2_id);
			preStmt->setInt(5, type);
			preStmt->setString(6, content);
			int row = preStmt->executeUpdate();
			if (row < 0) {
				return false;
			}
		}
	}
	catch (sql::SQLException& e) {
		std::cerr << "Excute sql command occure Error! " << std::endl;
		std::cerr << "Error Code: " << e.getErrorCode() << ", SQLState: "
			<< e.getSQLState() << ", what: " << e.what() << std::endl;
		return false;
	}
	return true;
}

bool MysqlDao::GetChatMessage(int64_t& lastId, const int64_t& thread_id, int pageSize, std::vector<std::shared_ptr<ChatDataBase>>& data_list, bool& loadMore){
	auto con = _pool->getConnection();
	Defer defer([&con, this]() {
		_pool->returnConnection(std::move(con));
		});
	try {
		if (con == nullptr) {
			return false;
		}

		std::unique_ptr<sql::PreparedStatement> preStmt(con->_con->prepareStatement(
			"SELECT * FROM chat_message WHERE thread_id = ? AND message_id > ? LIMIT ?"));
		preStmt->setInt64(1, thread_id);
		preStmt->setInt64(2, lastId);
		preStmt->setInt(3, pageSize + 1);
		std::unique_ptr<sql::ResultSet> res(preStmt->executeQuery());
		while (res->next()) {
			auto data = std::make_shared<ChatDataBase>(
							res->getInt64("message_id"), 
							thread_id, 
							res->getInt("type"), 
							res->getString("content"), res->getInt("sender_id"));
			data_list.push_back(data);
		}

		if (data_list.size() > pageSize) {
			loadMore = true;
			data_list.pop_back();
		}

		if (!data_list.empty()) {
			lastId = data_list.back()->GetMsgId();
		}
	}
	catch (sql::SQLException& e) {
		std::cerr << "Excute sql command occure Error! " << std::endl;
		std::cerr << "Error Code: " << e.getErrorCode() << ", SQLState: "
			<< e.getSQLState() << ", what: " << e.what() << std::endl;
		return false;
	}
	return true;
}
