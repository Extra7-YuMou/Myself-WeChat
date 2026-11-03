#include "FileSystem.h"
#include "ConfigMgr.h"

FileSystem::FileSystem() {
	for (int i = 0; i < FILE_WORKER_COUNT; i++) {
		_workers.push_back(std::make_shared<FileWorker>());
	}

	for (int i = 0; i < FILE_WORKER_COUNT; i++) {
		_download_workers.push_back(std::make_shared<DownLoadWorker>());
	}
}

void FileSystem::PostTask(std::shared_ptr<FileTask> task, short index){
	_workers[index]->PostTask(task);
}

void FileSystem::PostDownLoadTask(std::shared_ptr<DownLoadTask> task, short index){
	_download_workers[index]->PostTask(task);
}

int64_t FileSystem::GetFileTotalSize(const std::string& dir_name, const std::string& file_name){
	auto targetDir = ConfigMgr::GetInstance()->getOutputPath();
	auto targetPath = (targetDir / dir_name / file_name).string();

	if (!std::filesystem::exists(targetPath)) {
		std::cout << "Taget file not exists, file path: " << targetPath;
		return 0;
	}

	std::ifstream in(targetPath, std::ios::binary);
	if (!in) {
		std::cout << "Open the file: " << targetPath << " failed";
		return 0;
	}

	in.seekg(0, std::ios::end);
	std::streamsize file_size = in.tellg();
	in.seekg(0, std::ios::beg);

	in.close();
	return file_size;
}
