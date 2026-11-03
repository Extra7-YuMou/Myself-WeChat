#include "MysqlPool.h"
#include "StatusConPool.h"

MysqlPool::MysqlPool(std::size_t poolSize, const std::string& url, const std::string& pwd, const std::string& schema, const std::string& user) : 
	_user(user), _poolSize(poolSize), _url(url), _pwd(pwd), _schema(schema), _b_stop(false){
	try {
		for (std::size_t i = 0; i < poolSize; i++) {
			sql::mysql::MySQL_Driver* driver = sql::mysql::get_driver_instance();
			auto* con(driver->connect(url, user, pwd));
			con->setSchema(schema);

			auto currentTime = std::chrono::system_clock().now().time_since_epoch();
			long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();

			_pool.push(std::make_unique<SqlConnection>(con, timestamp));
		}

		_check_thread = std::thread([this]() {
			while (!_b_stop) {
				checkConnection();
				std::this_thread::sleep_for(std::chrono::seconds(60));
			}
			});
	}
	catch (sql::SQLException& e) {
		std::cout << "MysqlPool init failed" << std::endl;
		std::cout << "Error: " << e.what() << std::endl;
	}
}

MysqlPool::~MysqlPool(){
	std::unique_lock<std::mutex> lock(_mtx);
	while (!_pool.empty()) {
		_pool.pop();
	}
}

std::unique_ptr<SqlConnection> MysqlPool::GetConnection() {
	std::unique_lock<std::mutex> lock(_mtx);
	_cond.wait(lock, [this]() {
		if (_b_stop) {
			return true;
		}
		return !_pool.empty();
		});

	if (_b_stop) {
		return nullptr;
	}

	auto con = std::move(_pool.front());
	_pool.pop();
	return con;
}

void MysqlPool::returnConnection(std::unique_ptr<SqlConnection> connect){
	std::unique_lock<std::mutex> lock(_mtx);
	_pool.push(std::move(connect));
	_cond.notify_one();
}

void MysqlPool::Close() {
	_b_stop = true;
	_cond.notify_all();
}

void MysqlPool::checkConnection(){
	// 获取当前时间戳
	auto currentTime = std::chrono::system_clock().now().time_since_epoch();
	long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();
	for (std::size_t i = 0; i < _poolSize; i++) {
		auto con = std::move(_pool.front());
		_pool.pop();
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
			// 创建新连接替换旧链接
			sql::mysql::MySQL_Driver* driver = sql::mysql::get_driver_instance();
			auto _con = driver->connect(_url, _user, _pwd);
			_con->setSchema(_schema);
			con->_con.reset(_con);
			con->_last_oper_time = timestamp;
		}
	}
}
