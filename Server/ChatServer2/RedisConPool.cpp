#include "RedisConPool.h"
#include <iostream>

RedisConPool::RedisConPool(std::size_t size, const std::string& host, const short& port, const std::string& pwd): 
	_b_stop(false), _host(host), _port(port), _pwd(pwd){
	for (std::size_t i = 0; i < size; i++) {
		auto* con = redisConnect(host.c_str(), port);
		if (con == nullptr || con->err != 0) {
			if (con == nullptr) {
				redisFree(con);
			}
			continue;
		}

		auto* reply = (redisReply*)redisCommand(con, "AUTH %s", pwd.c_str());
		if (reply == nullptr || reply->type == REDIS_REPLY_ERROR) {
			std::cout << "Redis认证失败" << std::endl;
			freeReplyObject(reply);
			continue;
		}
		std::cout << "Redis认证成功" << std::endl;
		freeReplyObject(reply);
		_pool.push(con);
	}
}

RedisConPool::~RedisConPool(){
	std::unique_lock<std::mutex> lock(_mtx);
	while (!_pool.empty()) {
		_pool.pop();
	}
}

redisContext* RedisConPool::getConnection(){
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

	auto* con = _pool.front();
	_pool.pop();
	return con;
}

void RedisConPool::returnConnection(redisContext* con){
	std::unique_lock<std::mutex> lock(_mtx);
	_pool.push(con);
	_cond.notify_one();
}

void RedisConPool::Close() {
	_b_stop = true;
	_cond.notify_all();
}