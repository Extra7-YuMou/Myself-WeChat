#include "DownLoadWorker.h"
#include "ConfigMgr.h"
#include "RedisMgr.h"
#include "LogicSystem.h"
#include "Base64.h"

DownLoadWorker::DownLoadWorker(): _b_stop(false){
	_worker_thread = std::thread([this]() {
		while (!_b_stop) {
			std::unique_lock<std::mutex> lock(_mtx);
			_cond.wait(lock, [this]() {
				if (_b_stop) {
					return true;
				}
				return !_task_que.empty();
				});

			if (_b_stop) {
				return;
			}

			auto task = _task_que.front();
			_task_que.pop();
			TaskCallBack(task);
		}
		});
}

void DownLoadWorker::PostTask(std::shared_ptr<DownLoadTask> task) {
	std::lock_guard<std::mutex> lock(_mtx);
	_task_que.push(task);
	_cond.notify_one();
}

void DownLoadWorker::TaskCallBack(std::shared_ptr<DownLoadTask> task) {
	auto targetDir = ConfigMgr::GetInstance()->getOutputPath();
	std::string targetPath;
	if (task->_dir_name.empty()) {
		targetPath = (targetDir / task->_name).string();
	}
	else {
		targetPath = (targetDir / task->_dir_name / task->_name).string();
	}

	std::cout << "TargetPath: " << targetPath << std::endl;
	Json::Value rtvalue;

	if (!std::filesystem::exists(targetPath)) {
		std::cout << "Target file not exists, file name: " << task->_name << std::endl;
		rtvalue["error"] = ErrorCodes::FileNotExists;
		task->_task_callback(rtvalue);
		return;
	}

	std::ifstream in(targetPath, std::ios::binary);
	if (!in) {
		std::cout << "Open the file failed, file name: " << task->_name << std::endl;
		rtvalue["error"] = ErrorCodes::FileReadPermissionFailed;
		task->_task_callback(rtvalue);
		return;
	}

	std::shared_ptr<FileInfo> fileInfo;
	if (task->_seq == 1) {
		// 获取文件大小
		in.seekg(0, std::ios::end);
		std::streamsize file_size = in.tellg();		
		in.seekg(0, std::ios::beg);

		fileInfo = std::make_shared<FileInfo>();
		fileInfo->_name = task->_name;
		fileInfo->_seq = task->_seq;
		fileInfo->_trans_size = task->_trans_size;
		fileInfo->_total_size = file_size;
		task->_total_size = file_size;
		bool _b_set = RedisMgr::GetInstance()->SetDownLoadInfo(task->_name, fileInfo);
		if (!_b_set) {
			rtvalue["error"] = ErrorCodes::ERR_REDIS;
			task->_task_callback(rtvalue);
			return;
		}
	}
	else {
		fileInfo = RedisMgr::GetInstance()->GetDownLoadInfo(task->_name);
		if (fileInfo == nullptr) {
			std::cout << "未成功获取文件下载信息, file name: " << task->_name << std::endl;
			rtvalue["error"] = ErrorCodes::ERR_REDIS;
			task->_task_callback(rtvalue);
			return;
		}

		fileInfo->_seq = task->_seq;
		fileInfo->_trans_size = task->_trans_size;
		task->_total_size = fileInfo->_total_size;
		
		std::cout << "文件下载进度: 文件名: " << task->_name
			<< ", 序列号: " << task->_seq << ", 进度: "
			<< task->_trans_size << "/" << task->_total_size << std::endl;
	}

	std::streamsize offset = (task->_seq - 1) * MAX_FILE_SIZE;
	if (offset > task->_total_size) {
		std::cout << "total_size: " << task->_total_size << std::endl;
		std::cout << "文件读取错误, 文件名: " << task->_name << std::endl;
		rtvalue["error"] = ErrorCodes::FileReadFailed;
		task->_task_callback(rtvalue);
		return;
	}
	in.seekg(offset);

	char buffer[MAX_FILE_SIZE];
	in.read(buffer, MAX_FILE_SIZE);

	std::streamsize bytes_read = in.gcount();
	std::cout << "bytes_read: " << bytes_read << std::endl;
	if (bytes_read <= 0) {
		std::cout << "文件读取失败, 文件名: " << task->_name << std::endl;
		rtvalue["error"] = ErrorCodes::FileReadFailed;
		task->_task_callback(rtvalue);
		return;
	}
	task->_trans_size += bytes_read;

	// 进行base64编码
	std::string originalData = std::string(buffer, bytes_read);
	std::string base64Data = base64_encode(originalData);

	std::streamsize current_size = offset + bytes_read;
	bool finished = (current_size >= task->_total_size);

	rtvalue["name"] = task->_name;
	rtvalue["seq"] = task->_seq;
	rtvalue["trans_size"] = bytes_read;
	rtvalue["total_size"] = task->_total_size;
	rtvalue["data"] = base64Data;
	rtvalue["finished"] = finished;

	if (finished) {
		std::cout << "文件下载完成, 文件名: " << task->_name << std::endl;
		RedisMgr::GetInstance()->DelDownLoadInfo(task->_name);
	}
	else {
		RedisMgr::GetInstance()->SetDownLoadInfo(task->_name, fileInfo);
	}

	task->_task_callback(rtvalue);
}