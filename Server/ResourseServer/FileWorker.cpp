#include "FileWorker.h"
#include "MysqlMgr.h"
#include "Global.h"
#include "RedisMgr.h"
#include "ResourceGrpcClient.h"

FileWorker::FileWorker(): _b_stop(false){
	InitHandlers();

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

FileWorker::~FileWorker(){
	_b_stop = true;
	_cond.notify_all();
	_worker_thread.join();
}

void FileWorker::PostTask(std::shared_ptr<FileTask> task) {
	std::unique_lock<std::mutex> lock(_mtx);
	_task_que.push(task);
	_cond.notify_one();
}

void FileWorker::InitHandlers() {
	_handlers[FileTaskType::UploadHeadIcon] = [this](std::shared_ptr<FileTask> task) {
		Json::Value rtvalue;
		bool success = MysqlMgr::GetInstance()->UpdateUserHeadIcon(task->_uid, task->_name);
		if (!success) {
			std::cout << "Update user head icon failed" << std::endl;
			rtvalue["error"] = ErrorCodes::UpdateUserInfo;
			if (task->_task_callback) {
				task->_task_callback(rtvalue);
				return;
			}
		}

		auto userInfo = MysqlMgr::GetInstance()->GetUserInfo(task->_uid);
		if (userInfo) {
			Json::Value userJson;
			userJson["uid"] = userInfo->_uid;
			userJson["name"] = userInfo->_name;
			userJson["nick"] = userInfo->_nick;
			userJson["email"] = userInfo->_email;
			userJson["icon"] = userInfo->_icon;
			userJson["sex"] = userInfo->_sex;
			userJson["desc"] = userInfo->_desc;

			std::string key = USER_BASE_INFO + std::to_string(userInfo->_uid);
			RedisMgr::GetInstance()->Set(key, userJson.toStyledString());

			rtvalue["error"] = ErrorCodes::SUCCESS;
			rtvalue["finish"] = true;
		}

		if (task->_task_callback) {
			task->_task_callback(rtvalue);
		}

		};

	_handlers[FileTaskType::UploadImgChatMsg] = [this](std::shared_ptr<FileTask> task) {
		Json::Value rtvalue;
		rtvalue["error"] = ErrorCodes::SUCCESS;
		rtvalue["finished"] = true;

		if (task->_task_callback) {
			task->_task_callback(rtvalue);
		}
		};

	_handlers[FileTaskType::UploadVideoCover] = [this](std::shared_ptr<FileTask> task) {
		Json::Value rtvalue;
		rtvalue["error"] = ErrorCodes::SUCCESS;
		rtvalue["finished"] = true;

		if (task->_task_callback) {
			task->_task_callback(rtvalue);
		}
		};

	_handlers[FileTaskType::UploadVideoBody] = [this](std::shared_ptr<FileTask> task) {
		Json::Value rtvalue;
		rtvalue["error"] = ErrorCodes::SUCCESS;
		rtvalue["finished"] = true;

		if (task->_task_callback) {
			task->_task_callback(rtvalue);
		}
		};
}

void FileWorker::TaskCallBack(std::shared_ptr<FileTask> task) {
	std::ofstream ofs;
	std::cout << "task file path: " << task->_file_path << std::endl;
	if (task->_seq == 1) {
		ofs.open(task->_file_path, std::ios::binary | std::ios::trunc);
	}
	else {
		ofs.open(task->_file_path, std::ios::binary | std::ios::app);
	}

	if (!ofs) {
		std::cout << "The file open failed" << std::endl;
		return;
	}

	ofs.write(task->_file_data.c_str(), task->_file_data.length());
	if (!ofs) {
		std::cout << "Write data to file failed" << std::endl;
		return;
	}

	std::cout << "Write data to file success" << std::endl;
	ofs.close();

	if (task->_finished) {
		auto iter_find = _handlers.find(task->_type);
		if (iter_find == _handlers.end()) {
			std::cout << "Handler not find" << std::endl;
			return;
		}
		iter_find->second(task);
		return;
	}

	Json::Value rtvalue;
	rtvalue["error"] = ErrorCodes::SUCCESS;
	rtvalue["name"] = task->_name;
	rtvalue["seq"] = task->_seq;
	rtvalue["trans_size"] = task->_trans_size;
	rtvalue["total_size"] = task->_total_size;
	rtvalue["finished"] = task->_finished;

	if (task->_task_callback) {
		task->_task_callback(rtvalue);
	}
}