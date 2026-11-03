#pragma once
#include "Singleton.h"
#include <iostream>
#include <memory>
#include <hiredis.h>
#include "RedisConPool.h"
#include "DistLock.h"

class FileInfo;
class RedisMgr : public Singleton<RedisMgr>, public std::enable_shared_from_this<RedisMgr> {
	friend class Singleton<RedisMgr>;
public:
	~RedisMgr();
	bool Get(const std::string& key, std::string& value);
	bool Set(const std::string& key, const std::string& value);
	bool LPush(const std::string& key, const std::string& value);
	bool LPop(const std::string& key, std::string& value);
	bool RPush(const std::string& key, std::string& value);
	bool RPop(const std::string& key, std::string& value);
	bool HSet(const std::string& key, const std::string& hkey, const std::string& value);
	bool HSet(const char* key, const char* hkey, const char* hvalue);
	std::string HGet(const std::string& key, const std::string& hkey);
	bool Del(const std::string& key);
	bool HDel(const std::string& key, const std::string& hkey);
	bool ExistsKey(const std::string& key);
	void Close();
	std::string acquireLock(const std::string& lockName, const int& lockTimeout, const int& acquireTimeout);
	bool releaseLock(const std::string& lockName, const std::string& identifier);
	bool SetFileInfo(const std::string& md5, std::shared_ptr<FileInfo> fileInfo);
	bool SetExp(const std::string& key, const std::string& value, int expried_time);
	bool SetDownLoadInfo(const std::string& key, std::shared_ptr<FileInfo> fileInfo);
	std::shared_ptr<FileInfo> GetDownLoadInfo(const std::string& key);
	bool DelDownLoadInfo(const std::string& key);
private:
	RedisMgr();

	redisReply* _reply;
	std::unique_ptr<RedisConPool> _pool;
};

