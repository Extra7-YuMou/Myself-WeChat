#include "MysqlConPool.h"
#include "RedisConPool.h"

MysqlConPool::MysqlConPool(std::size_t size, const std::string& url, const std::string& user, const std::string& schema, const std::string& pwd) :
	_size(size), _url(url), _user(user), _schema(schema), _pwd(pwd), _b_stop(false) {
	try {
		for (std::size_t i = 0; i < size; i++) {
			sql::mysql::MySQL_Driver* driver = sql::mysql::get_driver_instance();
			auto* con(driver->connect(url, user, pwd));
			con->setSchema(schema);

			auto currentTime = std::chrono::system_clock().now().time_since_epoch();
			long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();

			_connections.push(std::make_unique<SqlConnection>(con, timestamp));
		}
	}
	catch (sql::SQLException& e) {
		std::cout << "MysqlConPool init failure, Error code: "
			<< e.getErrorCode() << ", message: " << e.what()
			<< ", SQLState: " << e.getSQLState() << std::endl;
	}
}

MysqlConPool::~MysqlConPool(){
	std::unique_lock<std::mutex> lock(_mtx);
	while (!_connections.empty()) {
		_connections.pop();
	}
}

std::unique_ptr<SqlConnection> MysqlConPool::GetConnection(){
	std::unique_lock<std::mutex> lock(_mtx);
	_cond.wait(lock, [this]() {
		if (_b_stop) {
			return true;
		}
		return !_connections.empty();
		});

	if (_b_stop) {
		return nullptr;
	}

	auto con = std::move(_connections.front());
	_connections.pop();
	return con;
}

void MysqlConPool::returnConnection(std::unique_ptr<SqlConnection> connection){
	std::unique_lock<std::mutex> lock(_mtx);
	_connections.push(std::move(connection));
	_cond.notify_one();
}

void MysqlConPool::Close() {
	_b_stop = true;
	_cond.notify_all();
}

void MysqlConPool::checkConnection(){
	std::unique_lock<std::mutex> lock(_mtx);
	auto currentTime = std::chrono::system_clock().now().time_since_epoch();
	long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();

	for (std::size_t i = 0; i < _size; i++) {
		auto con = std::move(_connections.front());
		_connections.pop();
		Defer defer([&con, this]() {
			returnConnection(std::move(con));
			});

		if (timestamp - con->_last_oper_time < 5) {
			continue;
		}

		try {
			std::unique_ptr<sql::Statement> stmt(con->_con->createStatement());
			stmt->executeQuery("SELECT 1");
			con->_last_oper_time = timestamp;
		}
		catch (sql::SQLException& e) {
			std::cout << "Error keeping connection alive: " << e.what() << std::endl;
			sql::mysql::MySQL_Driver* driver = sql::mysql::get_driver_instance();
			auto new_con = driver->connect(_url, _user, _pwd);
			new_con->setSchema(_schema);
			con->_con.reset(new_con);
			con->_last_oper_time = timestamp;
		}
	}
}
