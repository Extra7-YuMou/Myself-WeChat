#pragma once
#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/exception.h>
#include <queue>
#include <mutex>
#include <iostream>
#include "Global.h"
#include <chrono>

class SqlConnection {
public:
	SqlConnection(sql::Connection* con, int64_t lasttime) : _con(con), _last_oper_time(lasttime){}
	std::shared_ptr<sql::Connection> _con;
	int64_t _last_oper_time;
};

class MysqlConPool{
public:
	MysqlConPool(std::size_t size, const std::string& user, const std::string& pwd, const std::string& url, const std::string& schema);
	std::unique_ptr<SqlConnection> getConnection();
	void returnConnection(std::unique_ptr<SqlConnection> _connection);
	void Close();
	~MysqlConPool();
private:
	void checkConnection();
	
	std::queue<std::unique_ptr<SqlConnection>> _pool;
	std::mutex _mtx;
	std::string _user;
	std::string _pwd;
	std::string _url;
	std::string _schema;
	std::size_t _poolSize;
	std::thread _check_thread;
	std::atomic<bool> _b_stop;
	std::condition_variable _cond;
};

