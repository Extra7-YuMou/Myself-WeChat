#include "LogicSystem.h"

LogicSystem::LogicSystem() {
	for (int i = 0; i < LOGIC_WORKER_COUNT; i++) {
		_workers.push_back(std::make_shared<LogicWorker>());
	}
}

void LogicSystem::PostToQue(std::shared_ptr<LogicNode> node, short index){
	_workers[index]->PostTask(node);
}

void LogicSystem::AddFileInfo(const std::string& md5, std::shared_ptr<FileInfo> fileInfo){
	{
		std::lock_guard<std::mutex> lock(_mtx);
		_md5_files[md5] = fileInfo;
	}
}

std::shared_ptr<FileInfo> LogicSystem::GetFileInfo(const std::string& md5){
	{
		std::lock_guard<std::mutex> lock(_mtx);
		auto iter_find = _md5_files.find(md5);
		if (iter_find == _md5_files.end()) {
			return nullptr;
		}

		return iter_find->second;
	}
}
