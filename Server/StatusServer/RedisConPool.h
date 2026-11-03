#pragma once
#include "Global.h"
#include "hiredis.h"

class RedisConPool{
public:
	RedisConPool(std::size_t size, const std::string& host, const int& port, const std::string& pwd);
	~RedisConPool();
	redisContext* GetConnection();
	void returnConnection(redisContext* connection);
	void Close();
private:
	std::size_t _size;
	std::string _host;
	int _port;
	std::queue<redisContext*> _connections;
	std::mutex _mtx;
	std::condition_variable _cond;
	std::atomic<bool> _b_stop;
};

