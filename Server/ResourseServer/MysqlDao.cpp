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

std::shared_ptr<UserInfo> MysqlDao::GetUserInfo(const int& uid){
	auto con = _pool->getConnection();
	Defer defer([&con, this]() {
		_pool->returnConnection(std::move(con));
		});

	try {
		if (con == nullptr) {
			return nullptr;
		}

		std::unique_ptr<sql::PreparedStatement> preStmt(con->_con->prepareStatement("SELECT * FROM `user` WHERE `uid` = ?"));
		preStmt->setInt(1, uid);
		std::unique_ptr<sql::ResultSet> res(preStmt->executeQuery());
		if (res->next()) {
			auto uid = res->getInt("uid");
			auto name = res->getString("name");
			auto nick = res->getString("nick");
			auto email = res->getString("email");
			auto icon = res->getString("icon");
			auto sex = res->getInt("sex");
			auto desc = res->getString("desc");
			auto userInfo = std::make_shared<UserInfo>(uid, name, nick, icon, email, sex, desc);
			return userInfo;
		}
	}
	catch (sql::SQLException& e) {
		std::cout << "Sql Error, state: " << e.getSQLState() << ", msg: " << e.what() << std::endl;
		return nullptr;
	}
	return nullptr;
}

bool MysqlDao::UpdateUserHeadIcon(const int& uid, const std::string& icon_path){
	auto con = _pool->getConnection();
	Defer defer([&con, this]() {
		_pool->returnConnection(std::move(con));
		});

	try {
		if (con == nullptr) {
			return false;
		}

		std::string update_str = "UPDATE `user` SET `icon` = ? WHERE `uid` = ?";
		std::unique_ptr<sql::PreparedStatement> preStmt(con->_con->prepareStatement(update_str));
		preStmt->setString(1, icon_path);
		preStmt->setInt(2, uid);
		int row = preStmt->executeUpdate();

		if (row < 0) {
			return false;
		}
	}
	catch (sql::SQLException& e) {
		std::cout << "Sql Error, state: " << e.getSQLState() << ", msg: " << e.what() << std::endl;
		return false;
	}
	return true;
}

