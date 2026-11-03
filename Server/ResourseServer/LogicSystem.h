#pragma once
#include "Singleton.h"
#include "LogicWorker.h"
#include "CSession.h"
#include <map>

class FileInfo {
public:
	FileInfo() : _name(""), _seq(0), _total_size(0), _trans_size(0), _file_path("") {}
	~FileInfo() = default;
	std::string _md5;
	std::string _name;
	int _seq;
	int _total_size;
	int64_t _trans_size;
	std::string _file_path;
};

class LogicSystem : public Singleton<LogicSystem>{
	friend class Singleton<LogicSystem>;
public:
	~LogicSystem() = default;
	void PostToQue(std::shared_ptr<LogicNode>, short);
	void AddFileInfo(const std::string& md5, std::shared_ptr<FileInfo>);
	std::shared_ptr<FileInfo> GetFileInfo(const std::string& md5);
private:
	LogicSystem();
	LogicSystem(const LogicSystem&) = delete;
	LogicSystem& operator=(const LogicSystem&) = delete;

	std::vector<std::shared_ptr<LogicWorker>> _workers;
	std::mutex _mtx;
	std::map<std::string, std::shared_ptr<FileInfo>> _md5_files;
};

