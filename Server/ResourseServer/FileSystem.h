#pragma once
#include "Singleton.h"
#include <iostream>
#include "FileWorker.h"
#include "DownLoadWorker.h"

class FileSystem : public Singleton<FileSystem>{
	friend class Singleton<FileSystem>;
public:
	~FileSystem() = default;
	void PostTask(std::shared_ptr<FileTask>, short);
	void PostDownLoadTask(std::shared_ptr<DownLoadTask>, short);
	int64_t GetFileTotalSize(const std::string& dir_name, const std::string& file_name);
private:
	FileSystem();
	FileSystem(const FileSystem&) = delete;
	FileSystem& operator=(const FileSystem&) = delete;

	std::vector<std::shared_ptr<FileWorker>> _workers;
	std::vector<std::shared_ptr<DownLoadWorker>> _download_workers;
};

