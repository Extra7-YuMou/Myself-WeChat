#include "MysqlDao.h"
#include "ConfigMgr.h"

MysqlDao::MysqlDao(){
	auto config = ConfigMgr::GetInstance();
	const auto& user = (*config)["Mysql"]["User"];
	const auto& pwd = (*config)["Mysql"]["Passwd"];
	const auto& port = (*config)["Mysql"]["Port"];
	const auto& host = (*config)["Mysql"]["Host"];
	const auto& schema = (*config)["Mysql"]["Schema"];
	_pool = std::make_unique<MysqlPool>(5, host + ":" + port, pwd, schema, user);
}

MysqlDao::~MysqlDao(){
	_pool->Close();
}

int MysqlDao::RegUser(const std::string& name, const std::string& email, const std::string& pwd){
	auto con = _pool->GetConnection();
	try {
		if (con == nullptr) {
			_pool->returnConnection(std::move(con));
			return false;
		}

		std::unique_ptr<sql::PreparedStatement> preStmt(con->_con->prepareStatement("CALL reg_user(?, ?, ?, @result)"));
		preStmt->setString(1, name);
		preStmt->setString(2, email);
		preStmt->setString(3, pwd);
		preStmt->execute();

		std::unique_ptr<sql::Statement> stmt(con->_con->createStatement());
		std::unique_ptr<sql::ResultSet> stmtRes(stmt->executeQuery("SELECT @result AS result"));
		Defer defer([&con, this]() {
			_pool->returnConnection(std::move(con));
			});
		if (stmtRes->next()) {
			int result = stmtRes->getInt("result");
			std::cout << "Result: " << result << std::endl;
			/*_pool->returnConnection(std::move(con));*/
			return result;
		}
		//_pool->returnConnection(std::move(con));
		return -1;
	}
	catch (sql::SQLException& e) {
		std::cerr << "Excute sql command occure Error! " << std::endl;
		std::cerr << "Error Code: " << e.getErrorCode() << ", SQLState: "
			<< e.getSQLState() << std::endl;
		return -1;
	}
	return 0;
}

bool MysqlDao::CheckEmail(const std::string& name, const std::string& email){
	auto con = _pool->GetConnection();
	try {
		if (con == nullptr) {
			_pool->returnConnection(std::move(con));
			return false;
		}

		std::unique_ptr<sql::PreparedStatement> preStmt(con->_con->prepareStatement("SELECT email FROM user WHERE name = ?"));
		preStmt->setString(1, name);

		std::unique_ptr<sql::ResultSet> stmtRes(preStmt->executeQuery());
		Defer defer([&con, this]() {
			_pool->returnConnection(std::move(con));
			});
		if (stmtRes->next()) {
			std::cout << "Check email: " << stmtRes->getString("email") << std::endl;
			if (email == stmtRes->getString("email")) {
				//_pool->returnConnection(std::move(con));
				return true;
			}
		}
		//_pool->returnConnection(std::move(con));
		return false;
	}
	catch (sql::SQLException& e) {
		std::cerr << "Excute sql command occure Error! " << std::endl;
		std::cerr << "Error Code: " << e.getErrorCode() << ", SQLState: "
			<< e.getSQLState() << std::endl;
		return false;
	}
	return false;
}

bool MysqlDao::ResetPwd(const std::string& name, const std::string& newPwd){
	auto con = _pool->GetConnection();
	try {
		if (con == nullptr) {
			_pool->returnConnection(std::move(con));
			return false;
		}

		std::unique_ptr<sql::PreparedStatement> preStmt(con->_con->prepareStatement("UPDATE user SET pwd = ? WHERE name = ?"));
		preStmt->setString(1, newPwd);
		preStmt->setString(2, name);

		int count = preStmt->executeUpdate();
		std::cout << "Update rows: " << count << std::endl;
		_pool->returnConnection(std::move(con));
		return true;
	}
	catch (sql::SQLException& e) {
		std::cerr << "Excute sql command occure Error! " << std::endl;
		std::cerr << "Error Code: " << e.getErrorCode() << ", SQLState: "
			<< e.getSQLState() << std::endl;
		return false;
	}
	return false;
}

bool MysqlDao::CheckPwd(const std::string& name, const std::string& pwd, UserInfo& userInfo){
	auto con = _pool->GetConnection();
	try {
		if (con == nullptr) {
			_pool->returnConnection(std::move(con));
			return false;
		}

		std::unique_ptr<sql::PreparedStatement> preStmt(con->_con->prepareStatement("SELECT * FROM user WHERE name = ?"));
		preStmt->setString(1, name);
		std::unique_ptr<sql::ResultSet> stmtRes(preStmt->executeQuery());
		Defer defer([&con, this]() {
			_pool->returnConnection(std::move(con));
			});
		if (stmtRes->next()) {
			std::cout << "Check pwd: " << stmtRes->getString("pwd") << std::endl;
			if (pwd == stmtRes->getString("pwd")) {
				userInfo.uid = stmtRes->getInt("uid");
				userInfo.name = stmtRes->getString("name");
				userInfo.email = stmtRes->getString("email");
				//_pool->returnConnection(std::move(con));
				return true;
			}
		}
		//_pool->returnConnection(std::move(con));
		return false;
	}
	catch (sql::SQLException& e) {
		std::cerr << "Excute sql command occure Error! " << std::endl;
		std::cerr << "Error Code: " << e.getErrorCode() << ", SQLState: "
			<< e.getSQLState() << std::endl;
		return false;
	}
	return false;
}


