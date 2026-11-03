#pragma once
#include "Global.h"

struct UserInfo {
	int uid;
	std::string name;
	std::string email;
	std::string pwd;
};

class SqlConnection {
public:
	SqlConnection(sql::Connection* con, int64_t lasttime) : _con(con), _last_oper_time(lasttime){}
	std::unique_ptr<sql::Connection> _con;
	int64_t _last_oper_time;
};

class MysqlConPool{
public:
	MysqlConPool(std::size_t size, const std::string& url, const std::string& user, const std::string& schema, const std::string& pwd);
	~MysqlConPool();
	std::unique_ptr<SqlConnection> GetConnection();
	void returnConnection(std::unique_ptr<SqlConnection> connection);
	void Close();
	void checkConnection();
private:
	std::string _user;
	std::string _url;
	std::string _schema;
	std::string _pwd;
	std::queue <std::unique_ptr< SqlConnection >> _connections;
	std::size_t _size;
	std::mutex _mtx;
	std::condition_variable _cond;
	std::atomic<bool> _b_stop;
	std::thread _check_thread;
};

