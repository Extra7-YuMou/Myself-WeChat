#pragma once
#include "CSession.h"
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <memory>
#include <fstream>
#include "Global.h"
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>

class DownLoadTask {
public:
	DownLoadTask(std::shared_ptr<CSession> session, std::function<void(const Json::Value&)> task_callback, 
		const std::string& name = "", const int& seq = 0,
		const int& trans_size = 0, const int64_t& total_size = 0, bool finished = false) :
		_session(session), _task_callback(task_callback), _name(name), _trans_size(trans_size), _total_size(total_size),
	_finished(finished), _dir_name(""){ }

	std::shared_ptr<CSession> _session;
	std::string _name;
	int _seq;
	int _trans_size;
	int64_t _total_size;
	std::string _file_data;
	bool _finished;
	std::string _dir_name;
	std::function<void(const Json::Value&)> _task_callback;
};

class DownLoadWorker{
public:
	DownLoadWorker();
	~DownLoadWorker() = default;
	void PostTask(std::shared_ptr<DownLoadTask>);
private:
	void TaskCallBack(std::shared_ptr<DownLoadTask>);
	std::queue<std::shared_ptr<DownLoadTask>> _task_que;
	std::thread _worker_thread;
	std::mutex _mtx;
	std::condition_variable _cond;
	std::atomic<bool> _b_stop;
};

