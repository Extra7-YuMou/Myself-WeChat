#pragma once
#include <boost/asio.hpp>
#include <memory>
#include <iostream>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/version.hpp>
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

namespace beast = boost::beast;
namespace http = beast::http;
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
	TokenInvalid = 10,
	UidInvalid = 11,
};

// 实现作用域退出时自动执行释放资源的操作
class Defer {
public:
	Defer(std::function<void()> func): _func(func){}

	~Defer() {
		_func();
	}
private:
	std::function<void()> _func;
};