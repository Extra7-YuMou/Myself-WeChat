#include "RedisConPool.h"
#include <iostream>

RedisConPool::RedisConPool(std::size_t size, const std::string& host, const short& port, const std::string& pwd): 
	_b_stop(false), _host(host), _port(port), _pwd(pwd), _size(size), _counter(0){
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

	_check_thread = std::thread([this]() {
		while (!_b_stop) {
			_counter++;
			if (_counter >= 60) {
				CheckConnect();
				_counter = 0;
			}

			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
		});
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

void RedisConPool::CheckConnect() {
	std::unique_lock<std::mutex> lock(_mtx);
	std::queue<redisContext*> new_pool;  // 临时队列，保证异常安全

	for (std::size_t i = 0; i < _size; ++i) {
		redisContext* context = _pool.front();
		_pool.pop();

		bool ok = false;
		redisReply* reply = (redisReply*)redisCommand(context, "PING");
		if (reply && reply->type != REDIS_REPLY_ERROR) {
			ok = true;
			freeReplyObject(reply);
			new_pool.push(context);
		}
		else {
			if (reply) freeReplyObject(reply);

			// 旧连接无效，释放它
			redisFree(context);

			// 尝试创建新连接
			redisContext* new_ctx = redisConnect(_host.c_str(), _port);
			if (new_ctx && !new_ctx->err) {
				redisReply* auth_reply = (redisReply*)redisCommand(new_ctx, "AUTH %s", _pwd.c_str());
				if (auth_reply && auth_reply->type != REDIS_REPLY_ERROR) {
					ok = true;
					freeReplyObject(auth_reply);
					new_pool.push(new_ctx);
				}
				else {
					if (auth_reply) freeReplyObject(auth_reply);
					redisFree(new_ctx);
					
					throw std::runtime_error("Failed to reconnect to Redis after ping failure");
				}
			}
			else {
				if (new_ctx) redisFree(new_ctx);
				throw std::runtime_error("Redis connection failed during check");
			}
		}
	}

	_pool.swap(new_pool);  // 替换为新的有效连接池
}