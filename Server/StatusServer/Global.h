#pragma once
#include <boost/asio.hpp>
#include <memory>
#include <iostream>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <map>
#include <vector>
#include <queue>
#include <unordered_map>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/exception.h>
#include <functional>

namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;
const std::string code_prefix = "code_";

enum ErrorCodes {
	SUCCESS = 0,
	ERR_JSON = 1,
	ERR_NETWORK = 2,
	ERR_RPCFialed = 3,
	VerifyExpired = 4,
	VerifyCodeErr = 5,
	UserExists = 6,
	EmailNotMatch = 7,
	ResetPwdFailed = 8,
	PasswdInvalid = 9,
	UidInvalid = 10,
	TokenInvalid = 11,
};

// 实现作用域退出时自动执行释放资源的操作
class Defer {
public:
	Defer(std::function<void()> func) : _func(func) {}

	~Defer() {
		_func();
	}
private:
	std::function<void()> _func;
};

#define USERTOKENPREFIX "utoken_"
#define LOGIN_COUNT "logincount"
#define LOCK_LOGIN_COUNT "lock_login_count"
#define LOCK_TIME_OUT 10
#define ACQUIRE_TIME_OUT 5