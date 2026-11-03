#pragma once
#include <iostream>
#include <string>
#include "CSession.h"
#include <fstream>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>

class FileTask {
public:
	FileTask(std::shared_ptr<CSession> session, std::function<void(const Json::Value& value)> callback) : _session(session),
		_name(""), _seq(0), _trans_size(0), _total_size(0), _file_path(""), _file_data(""), _task_callback(callback), 
		_type(FileTaskType::NoType), _finished(false), _uid(0){ }
	~FileTask() = default;
	std::shared_ptr<CSession> _session;
	int _uid;
	std::string _name;
	int _seq;
	int _trans_size;
	int64_t _total_size;
	std::string _file_path;
	std::string _file_data;
	FileTaskType _type;
	bool _finished;
	std::function<void(const Json::Value& json)> _task_callback;
};

class FileWorker{
public:
	FileWorker();
	~FileWorker();
	void PostTask(std::shared_ptr<FileTask>);
private:
	void InitHandlers();
	void TaskCallBack(std::shared_ptr<FileTask>);

	std::thread _worker_thread;
	std::mutex _mtx;
	std::queue<std::shared_ptr<FileTask>> _task_que;
	std::condition_variable _cond;
	std::atomic<bool> _b_stop;
	std::map<FileTaskType, std::function<void(std::shared_ptr<FileTask>)>> _handlers;
};

