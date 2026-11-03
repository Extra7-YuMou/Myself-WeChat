#include "RedisConnectionPool.h"
#include "MysqlPool.h"

RedisConnectionPool::RedisConnectionPool(std::size_t poolSize_, const char* host, int port, const char* passwd) : _b_stop(false),
	_poolSize(poolSize_) , _host(host), _port(port) {
	for (std::size_t i = 0; i < poolSize_; i++) {
		auto* connect = redisConnect(host, port);
		if (connect == nullptr || connect->err != 0) {
			if (connect != nullptr) {
				redisFree(connect);
			}
			continue;
		}

		auto* reply = (redisReply*)redisCommand(connect, "AUTH %s", passwd);
		if (reply == nullptr || reply->type == REDIS_REPLY_ERROR) {
			std::cout << "Redis认证失败" << std::endl;
			freeReplyObject(reply);
			continue;
		}

		std::cout << "Redis认证成功" << std::endl;
		freeReplyObject(reply);
		_connections.push(connect);
	}
}

RedisConnectionPool::~RedisConnectionPool(){
	std::unique_lock<std::mutex> lock(_mtx);
	while (!_connections.empty()) {
		_connections.pop();
	}
}

redisContext* RedisConnectionPool::GetConnection(){
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
	auto* connect = _connections.front();
	_connections.pop();
	return connect;
}

void RedisConnectionPool::returnConnection(redisContext* connect){
	std::unique_lock<std::mutex> lock(_mtx);
	if (_b_stop) {
		return;
	}
	_connections.push(connect);
	_cond.notify_one();
}

void RedisConnectionPool::Close() {
	_b_stop = true;
	_cond.notify_all();
}

