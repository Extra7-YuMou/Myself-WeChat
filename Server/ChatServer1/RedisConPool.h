#pragma once
#include "hiredis.h"
#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>

class RedisConPool{
public:
	RedisConPool(std::size_t size, const std::string& host, const short& port, const std::string& pwd);
	~RedisConPool();
	redisContext* getConnection();
	void returnConnection(redisContext* con);
	void Close();
private:
	void CheckConnect();

	std::queue<redisContext*> _pool;
	std::mutex _mtx;
	std::condition_variable _cond;
	std::atomic<bool> _b_stop;
	std::string _host;
	short _port;
	std::string _pwd;
	std::size_t _size;
	int _counter;
	std::thread _check_thread;
};

