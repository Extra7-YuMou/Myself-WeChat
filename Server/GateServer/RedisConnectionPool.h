#pragma once
#include <hiredis.h>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <queue>
#include <atomic>

class RedisConnectionPool{
public:
	RedisConnectionPool(std::size_t poolSize_, const char* host, int port, const char* passwd);
	~RedisConnectionPool();
	redisContext* GetConnection();
	void returnConnection(redisContext*);
	void Close();
private:
	std::atomic<bool> _b_stop;
	std::queue<redisContext*> _connections;
	std::mutex _mtx;
	std::condition_variable _cond;
	const char* _host;
	int _port;
	std::size_t _poolSize;
};

