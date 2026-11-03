#include "MysqlConPool.h"

MysqlConPool::MysqlConPool(std::size_t size, const std::string& user, const std::string& pwd, const std::string& url, const std::string& schema):
	_poolSize(size), _user(user), _pwd(pwd), _url(url), _schema(schema), _b_stop(false){
	try {
		for (std::size_t i = 0; i < size; i++) {
			sql::mysql::MySQL_Driver* driver = sql::mysql::get_driver_instance();
			auto* con = driver->connect(url, user, pwd);
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

std::unique_ptr<SqlConnection> MysqlConPool::getConnection(){
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

void MysqlConPool::returnConnection(std::unique_ptr<SqlConnection> _connection){
	std::unique_lock<std::mutex> lock(_mtx);
	_pool.push(std::move(_connection));
	_cond.notify_one();
}

MysqlConPool::~MysqlConPool(){
	std::unique_lock<std::mutex> lock(_mtx);
	while (!_pool.empty()) {
		_pool.pop();
	}
	if (_check_thread.joinable()) {
		_check_thread.join();
	}
}

void MysqlConPool::checkConnection(){
	auto currentTime = std::chrono::system_clock().now().time_since_epoch();
	long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();

	for (std::size_t i = 0; i < _poolSize; i++) {
		auto con = std::move(_pool.front());
		_pool.pop();
		Defer defer([this, &con]() {
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
			auto* driver = sql::mysql::get_driver_instance();
			auto* new_con = driver->connect(_url, _user, _pwd);
			new_con->setSchema(_schema);
			con->_con.reset(new_con);
			con->_last_oper_time = timestamp;
		}
	}
}

void MysqlConPool::Close() {
	_b_stop = true;
	_cond.notify_all();
}