#include "RedisMgr.h"
#include "ConfigMgr.h"
#include "Global.h"
#include "LogicSystem.h"

RedisMgr::RedisMgr() {
	auto config = ConfigMgr::GetInstance();
	std::string host = (*config)["Redis"]["Host"];
	int port = std::atoi((*config)["Redis"]["Port"].c_str());
	std::string passwd = (*config)["Redis"]["Passwd"];
	_pool = std::unique_ptr<RedisConPool>(new RedisConPool(10, host.c_str(), port, passwd.c_str()));
}

RedisMgr::~RedisMgr() {
	Close();
}

bool RedisMgr::Get(const std::string& key, std::string& value) {
	auto _connect = _pool->getConnection();
	Defer defer([this, &_connect]() {
		_pool->returnConnection(_connect);
		});

	if (_connect == nullptr) {
		return false;
	}
	_reply = (redisReply*)redisCommand(_connect, "GET %s", key.c_str());
	if (_reply == nullptr) {
		std::cout << "Excute command [ GET " << key << " ] failed, _reply is nullptr" << std::endl;
		freeReplyObject(_reply);
		return false;
	}

	if (_reply->type != REDIS_REPLY_STRING) {
		std::cout << "Excute command [ GET " << key << " ] failed, _reply->type != REDIS_REPLT_STRING" << std::endl;
		freeReplyObject(_reply);
		return false;
	}

	value = _reply->str;
	freeReplyObject(_reply);
	std::cout << "Excute command [ GET " << key << " ] success" << std::endl;
	return true;
}

bool RedisMgr::Set(const std::string& key, const std::string& value) {
	auto _connect = _pool->getConnection();
	Defer defer([this, &_connect]() {
		_pool->returnConnection(_connect);
		});

	if (_connect == nullptr) {
		return false;
	}
	_reply = (redisReply*)redisCommand(_connect, "SET %s %s", key.c_str(), value.c_str());
	if (_reply == nullptr) {
		std::cout << "Excute command [ SET " << key << " " << value << " ] failed" << std::endl;
		freeReplyObject(_reply);
		return false;
	}

	if (!(_reply->type == REDIS_REPLY_STATUS && (strcmp(_reply->str, "OK") == 0 || strcmp(_reply->str, "ok") == 0))) {
		std::cout << "Excute command [ SET " << key << " " << value << " ] failed" << std::endl;
		freeReplyObject(_reply);
		return false;
	}

	std::cout << "Excute command [ SET " << key << " " << value << " ] success" << std::endl;
	freeReplyObject(_reply);
	return true;
}

bool RedisMgr::LPush(const std::string& key, const std::string& value) {
	auto _connect = _pool->getConnection();
	Defer defer([this, &_connect]() {
		_pool->returnConnection(_connect);
		});

	if (_connect == nullptr) {
		return false;
	}
	_reply = (redisReply*)redisCommand(_connect, "LPUSH %s %s", key.c_str(), value.c_str());
	if (_reply == nullptr) {
		std::cout << "Excute command [ LPUSH " << key << " " << value << " ] failed" << std::endl;
		freeReplyObject(_reply);
		return false;
	}

	if (_reply->type != REDIS_REPLY_INTEGER || _reply->integer <= 0) {
		std::cout << "Excute command [ LPUSH " << key << " " << value << " ] failed" << std::endl;
		freeReplyObject(_reply);
		return false;
	}
	std::cout << "Excute command [ LPUSH " << key << " " << value << " ] success" << std::endl;
	freeReplyObject(_reply);
	return true;
}

bool RedisMgr::LPop(const std::string& key, std::string& value) {
	auto _connect = _pool->getConnection();
	Defer defer([this, &_connect]() {
		_pool->returnConnection(_connect);
		});

	if (_connect == nullptr) {
		return false;
	}
	_reply = (redisReply*)redisCommand(_connect, "LPOP %s", key.c_str());
	if (_reply == nullptr || _reply->type == REDIS_REPLY_NIL) {
		std::cout << "Excute command [ LPOP " << key << " ] failed" << std::endl;
		freeReplyObject(_reply);
		return false;
	}

	value = _reply->str;
	std::cout << "Excute command [ LPOP " << key << " ] success" << std::endl;
	freeReplyObject(_reply);
	return true;
}

bool RedisMgr::RPush(const std::string& key, std::string& value) {
	auto _connect = _pool->getConnection();
	Defer defer([this, &_connect]() {
		_pool->returnConnection(_connect);
		});

	if (_connect == nullptr) {
		return false;
	}
	_reply = (redisReply*)redisCommand(_connect, "RPUSH %s %s", key.c_str(), value.c_str());
	if (_reply == nullptr) {
		std::cout << "Excute command [ RPUSH " << key << " " << value << " ] failure" << std::endl;
		freeReplyObject(_reply);
		return false;
	}

	if (_reply->type != REDIS_REPLY_INTEGER || _reply->integer <= 0) {
		std::cout << "Excute command [ RPUSH " << key << " " << value << " ] failure" << std::endl;
		freeReplyObject(_reply);
		return false;
	}

	std::cout << "Excute command [ RPUSH " << key << " " << value << " ] success" << std::endl;
	freeReplyObject(_reply);
	return true;
}

bool RedisMgr::RPop(const std::string& key, std::string& value) {
	auto _connect = _pool->getConnection();
	Defer defer([this, &_connect]() {
		_pool->returnConnection(_connect);
		});

	if (_connect == nullptr) {
		return false;
	}
	_reply = (redisReply*)redisCommand(_connect, "RPOP %s", key.c_str());
	if (_reply == nullptr || _reply->type == REDIS_REPLY_NIL) {
		std::cout << "Excute command [ RPOP " << key << " ] failure" << std::endl;
		freeReplyObject(_reply);
		return false;
	}

	value = _reply->str;
	std::cout << "Excute command [ RPOP " << key << " ] success" << std::endl;
	freeReplyObject(_reply);
	return true;
}

bool RedisMgr::HSet(const std::string& key, const std::string& hkey, const std::string& value) {
	auto _connect = _pool->getConnection();
	Defer defer([this, &_connect]() {
		_pool->returnConnection(_connect);
		});

	if (_connect == nullptr) {
		return false;
	}
	_reply = (redisReply*)redisCommand(_connect, "HSET %s %s %s", key.c_str(), hkey.c_str(), value.c_str());
	if (_reply == nullptr || _reply->type != REDIS_REPLY_INTEGER) {
		std::cout << "Excute command [ HSET " << key << " " << hkey << " " << value << " ] failure" << std::endl;
		freeReplyObject(_reply);
		return false;
	}

	std::cout << "Excute command [ HSET " << key << " " << hkey << " " << value << " ] success" << std::endl;
	freeReplyObject(_reply);
	return true;
}

bool RedisMgr::HSet(const char* key, const char* hkey, const char* hvalue) {
	auto _connect = _pool->getConnection();
	Defer defer([this, &_connect]() {
		_pool->returnConnection(_connect);
		});

	if (_connect == nullptr) {
		return false;
	}
	const char* argv[4];
	std::size_t argvlen[4];
	argv[0] = "HSET";
	argvlen[0] = 4;
	argv[1] = key;
	argvlen[1] = strlen(key);
	argv[2] = hkey;
	argvlen[2] = strlen(hkey);
	argv[3] = hvalue;
	argvlen[3] = strlen(hvalue);
	_reply = (redisReply*)redisCommandArgv(_connect, 4, argv, argvlen);
	if (_reply == nullptr || _reply->type != REDIS_REPLY_INTEGER) {
		std::cout << "Excute command [ HSET " << key << " " << hkey << " " << hvalue << " ] failure" << std::endl;
		freeReplyObject(_reply);
		return false;
	}

	std::cout << "Excute command [ HSET " << key << " " << hkey << " " << hvalue << " ] success" << std::endl;
	freeReplyObject(_reply);
	return true;
}

std::string RedisMgr::HGet(const std::string& key, const std::string& hkey) {
	auto _connect = _pool->getConnection();
	Defer defer([this, &_connect]() {
		_pool->returnConnection(_connect);
		});

	if (_connect == nullptr) {
		return "";
	}
	const char* argv[3];
	std::size_t argvlen[3];
	argv[0] = "HGET";
	argvlen[0] = 4;
	argv[1] = key.c_str();
	argvlen[1] = key.length();
	argv[2] = hkey.c_str();
	argvlen[2] = hkey.length();
	_reply = (redisReply*)redisCommandArgv(_connect, 3, argv, argvlen);
	if (_reply == nullptr || _reply->type == REDIS_REPLY_NIL) {
		std::cout << "Excute command [ HGET " << key << " " << hkey << " ] failure" << std::endl;
		freeReplyObject(_reply);
		return std::string();
	}

	std::string str = _reply->str;
	std::cout << "Excute command [ HGET " << key << " " << hkey << " ] success" << std::endl;
	freeReplyObject(_reply);
	return str;
}

bool RedisMgr::Del(const std::string& key) {
	auto _connect = _pool->getConnection();
	Defer defer([this, &_connect]() {
		_pool->returnConnection(_connect);
		});

	if (_connect == nullptr) {
		return false;
	}
	_reply = (redisReply*)redisCommand(_connect, "DEL %s", key.c_str());
	if (_reply == nullptr || _reply->type != REDIS_REPLY_INTEGER) {
		std::cout << "Excute command [ DEL " << key << " ] failure" << std::endl;
		freeReplyObject(_reply);
		_pool->returnConnection(_connect);
		return false;
	}

	std::cout << "EXcute command [ DEL " << key << " ] success" << std::endl;
	freeReplyObject(_reply);
	return true;
}

bool RedisMgr::HDel(const std::string& key, const std::string& hkey){
	auto _connect = _pool->getConnection();
	Defer defer([this, &_connect]() {
		_pool->returnConnection(_connect);
		});

	if (_connect == nullptr) {
		return false;
	}
	redisReply* reply = (redisReply*)redisCommand(_connect, "HDEL %s %s", key.c_str(), hkey.c_str());
	if (reply == nullptr) {
		std::cerr << "HDEL command failed" << std::endl;
		return false;
	}

	bool success = false;
	if (reply->type == REDIS_REPLY_INTEGER) {
		success = reply->integer > 0;
	}

	freeReplyObject(reply);
	return success;

}

bool RedisMgr::ExistsKey(const std::string& key) {
	auto _connect = _pool->getConnection();
	Defer defer([this, &_connect]() {
		_pool->returnConnection(_connect);
		});

	if (_connect == nullptr) {
		return false;
	}
	_reply = (redisReply*)redisCommand(_connect, "exists %s", key.c_str());
	if (_reply == nullptr || _reply->type != REDIS_REPLY_INTEGER || _reply->integer <= 0) {
		std::cout << "Excute command [ EXISTS " << key << " ] failure" << std::endl;
		freeReplyObject(_reply);
		_pool->returnConnection(_connect);
		return false;
	}

	std::cout << "Excute command [ EXISTS " << key << " ] success" << std::endl;
	freeReplyObject(_reply);
	return true;
}

void RedisMgr::Close() {
	_pool->Close();
}

std::string RedisMgr::acquireLock(const std::string& lockName, const int& lockTimeout, const int& acquireTimeout){
	auto* context = _pool->getConnection();
	if (context == nullptr) {
		_pool->returnConnection(context);
		return std::string();
	}

	Defer defer([&context, this]() {
		_pool->returnConnection(context);
		});

	std::string identifier = DistLock::Inst().acquireLock(context, lockName, lockTimeout, acquireTimeout);
	if (!identifier.empty()) {
		return identifier;
	}
	return std::string();
}

bool RedisMgr::releaseLock(const std::string& lockName, const std::string& identifier){
	auto* context = _pool->getConnection();
	if (context == nullptr) {
		_pool->returnConnection(context);
		return false;
	}

	Defer defer([&context, this]() {
		_pool->returnConnection(context);
		});

	bool _b_release = DistLock::Inst().releaseLock(context, lockName, identifier);
	if (_b_release) {
		return true;
	}
	return false;
}

bool RedisMgr::SetFileInfo(const std::string& md5, std::shared_ptr<FileInfo> fileInfo){
	Json::Value root;
	root["md5"] = fileInfo->_md5;
	root["name"] = fileInfo->_name;
	root["seq"] = fileInfo->_seq;
	root["trans_size"] = fileInfo->_trans_size;
	root["total_size"] = fileInfo->_total_size;
	std::string key = FILEUPLOADPREFIX + fileInfo->_md5;
	bool success = SetExp(key, root.toStyledString(), 3600);
	return success;
}

bool RedisMgr::SetExp(const std::string& key, const std::string& value, int expried_seconds){
	auto con = _pool->getConnection();
	if (con == nullptr) {
		return false;
	}
	Defer defer([&con, this]() {
		_pool->returnConnection(std::move(con));
		});

	redisReply* reply = (redisReply*)redisCommand(con, "SETEX %s %d %s", key.c_str(), expried_seconds, value.c_str());
	if (reply == nullptr) {
		std::cout << "Execute command [ SETEX " << key << " " << expried_seconds << " "
			<< value << " ] failed" << std::endl;
		return false;
	}

	if (!(reply->type == REDIS_REPLY_STATUS && (strcmp(reply->str, "ok") == 0 || strcmp(reply->str, "OK") == 0))) {
		std::cout << "Execute command [ SETEX " << key << " " << expried_seconds << " "
			<< value << " ] failed" << std::endl;
		return false;
	}

	freeReplyObject(reply);
	std::cout << "Execute command [ SETEX " << key << " " << expried_seconds << " "
		<< value << " ] success" << std::endl;
	return true;
}

bool RedisMgr::SetDownLoadInfo(const std::string& key, std::shared_ptr<FileInfo> fileInfo){
	auto con = _pool->getConnection();
	if (con == nullptr) {
		return false;
	}
	Defer defer([&con, this]() {
		_pool->returnConnection(std::move(con));
		});

	Json::Value value;
	value["name"] = fileInfo->_name;
	value["seq"] = fileInfo->_seq;
	value["trans_size"] = fileInfo->_trans_size;
	value["total_size"] = fileInfo->_total_size;
	auto value_str = value.toStyledString();

	std::string key_str = FILEDOWNLOADPREFIX + key;
	redisReply* reply = (redisReply*)redisCommand(con, "SETEX %s %d %s", key_str.c_str(),
		REDIS_FILEINFO_EXPRIED_TIME, value_str.c_str());
	if (reply == nullptr) {
		std::cout << "Execute command [ SET " << key_str << " " << REDIS_FILEINFO_EXPRIED_TIME
			<< " " << value_str << " ] failed" << std::endl;
		return false;
	}
	if (!(reply->type == REDIS_REPLY_STATUS && (strcmp(reply->str, "ok") == 0 || strcmp(reply->str, "OK") == 0))) {
		std::cout << "Execute command [ SET " << key_str << " " << REDIS_FILEINFO_EXPRIED_TIME
			<< " " << value_str << " ] failed" << std::endl;
		freeReplyObject(reply);
		return false;
	}

	freeReplyObject(reply);
	std::cout << "Execute command [ SET " << key_str << " " << REDIS_FILEINFO_EXPRIED_TIME
		<< " " << value_str << " ] success" << std::endl;
	return true;
}

std::shared_ptr<FileInfo> RedisMgr::GetDownLoadInfo(const std::string& key){
	std::string value_str;
	Get(FILEDOWNLOADPREFIX + key, value_str);
	Json::Reader reader;
	Json::Value value;
	reader.parse(value_str, value);

	auto fileInfo = std::make_shared<FileInfo>();
	fileInfo->_name = value["name"].asString();
	fileInfo->_seq = value["seq"].asInt();
	fileInfo->_trans_size = value["trans_size"].asInt();
	fileInfo->_total_size = value["total_size"].asInt64();
	return fileInfo;
}

bool RedisMgr::DelDownLoadInfo(const std::string& key){
	std::string key_str = FILEDOWNLOADPREFIX + key;
	return Del(key_str);
}

