#pragma once
#include "Global.h"

struct UserInfo {
	int uid;
	std::string name;
	std::string pwd;
	std::string email;
};

class SqlConnection {
public:
	SqlConnection(sql::Connection* con, int64_t lasttime) : _con(con), _last_oper_time(lasttime){}
	std::unique_ptr<sql::Connection> _con;
	int64_t _last_oper_time;
};

class MysqlPool{
public:
	MysqlPool(std::size_t poolSize, const std::string& url, const std::string& pwd, const std::string& schema, const std::string& user);
	~MysqlPool();
	std::unique_ptr<SqlConnection> GetConnection();
	void returnConnection(std::unique_ptr<SqlConnection> connect);
	void Close();
	void checkConnection();
private:
	std::queue<std::unique_ptr<SqlConnection>> _pool;
	std::mutex _mtx;
	std::condition_variable _cond;
	std::atomic<bool> _b_stop;
	std::size_t _poolSize;
	std::string _user;
	std::string _pwd;
	std::string _url;
	std::string _schema;
	std::thread _check_thread;
};

