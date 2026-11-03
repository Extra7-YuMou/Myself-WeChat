#include "LogicWorker.h"
#include "Global.h"
#include "LogicSystem.h"
#include "FileSystem.h"
#include "ConfigMgr.h"
#include "Base64.h"
#include "RedisMgr.h"
#include "MysqlMgr.h"
#include "ResourceGrpcClient.h"

LogicWorker::LogicWorker(): _b_stop(false){
	InitHandlers();

	_worker_thread = std::thread([this]() {
		while (!_b_stop) {
			std::unique_lock<std::mutex> lock(_mtx);
			_cond.wait(lock, [this]() {
				if (_b_stop) {
					return true;
				}
				return !_logic_que.empty();
				});

			if (_b_stop) {
				return;
			}

			auto node = _logic_que.front();
			_logic_que.pop();
			TaskCallBack(node);
		}
		});
}

void LogicWorker::PostTask(std::shared_ptr<LogicNode> node){
	std::lock_guard<std::mutex> lock(_mtx);
	_logic_que.push(node);
	_cond.notify_one();
}

LogicWorker::~LogicWorker(){
	_b_stop = true;
	_cond.notify_all();
	_worker_thread.join();
}

void LogicWorker::InitHandlers(){
	_handlers[MSG_UPLOAD_HEAD_ICON_REQ] = [this](std::shared_ptr<CSession> session, const short& id, const std::string& msg_data) {
		HandleUploadHeadIcon(session, id, msg_data);
		};

	/*_handlers[MSG_SYNC_UPLOAD_FILE_REQ] = [this](std::shared_ptr<CSession> session, const short& id, const std::string& msg_data) {
		HandleSyncUploadFile(session, id, msg_data);
		};*/
	_handlers[MSG_DOWNLOAD_FILE_REQ] = [this](std::shared_ptr<CSession> session, const short& id, const std::string& msg_data) {
		HandleDownLoadFile(session, id, msg_data);
		};

	_handlers[MSG_UPLOAD_IMG_CHAT_MSG_REQ] = [this](std::shared_ptr<CSession> session, const short& id, const std::string& msg_data) {
		HandleUploadChatImgMsg(session, id, msg_data);
		};

	_handlers[MSG_DOWNLOAD_IMG_CHAT_MSG_REQ] = [this](std::shared_ptr<CSession> session, const short& id, const std::string& msg_data) {
		HandleDownloadChatImgMsg(session, id, msg_data);
		};

	_handlers[MSG_UPLOAD_VIDEO_CHAT_MSG_REQ] = [this](std::shared_ptr<CSession> session, const short& id, const std::string& msg_data) {
		HandleUploadChatVideoMsg(session, id, msg_data);
		};

	_handlers[MSG_DOWNLOAD_VIDEO_COVER_CHAT_MSG_REQ] = [this](std::shared_ptr<CSession> session, const short& id, const std::string& msg_data) {
		HandleDownloadChatVideoCover(session, id, msg_data);
		};

	_handlers[MSG_DOWNLOAD_VIDEO_BODY_CHAT_MSG_REQ] = [this](std::shared_ptr<CSession> session, const short& id, const std::string& msg_data) {
		HandleDownloadChatVideoBody(session, id, msg_data);
		};
}

void LogicWorker::TaskCallBack(std::shared_ptr<LogicNode> node){
	auto iter_find = _handlers.find(node->_recv_node->_msg_id);
	if (iter_find == _handlers.end()) {
		std::cout << "Can't find callback" << std::endl;
		return;
	}
	iter_find->second(node->_session, node->_recv_node->_msg_id, 
		std::string(node->_recv_node->_msg, node->_recv_node->_total_len));
}

void LogicWorker::HandleUploadHeadIcon(std::shared_ptr<CSession> session, const short& id, const std::string& msg_data){
	Json::Value root, rtvalue;
	Json::Reader reader;
	reader.parse(msg_data, root);

	auto md5 = root["md5"].asString();
	auto name = root["name"].asString();
	auto seq = root["seq"].asInt();
	auto trans_size = root["trans_size"].asInt();
	auto total_size = root["total_size"].asInt64();
	auto originalData = root["data"].asString();
	auto uid = root["uid"].asInt();
	auto token = root["token"].asString();
	auto finished = root["finished"].asBool(); 

	std::string file_data = base64_decode(originalData);
	std::string targetPath;
	std::shared_ptr<FileInfo> fileInfo;

	// 第一个包验证token
	if (seq == 1) {
		std::string key = USERTOKENPREFIX + std::to_string(uid);
		std::string value;
		bool _b_get = RedisMgr::GetInstance()->Get(key, value);
		if (!_b_get) {
			std::cout << "The uid invalid, uid: " << uid << std::endl;
			rtvalue["error"] = ErrorCodes::UidInvalid;
			session->Send(rtvalue.toStyledString(), MSG_UPLOAD_HEAD_ICON_RSP);
			return;
		}
		std::cout << "value: " << value << ", token: " << token << std::endl;
		if (value != token) {
			std::cout << "The token invalid, token: " << token << std::endl;
			rtvalue["error"] = ErrorCodes::TokenInvalid;
			session->Send(rtvalue.toStyledString(), MSG_UPLOAD_HEAD_ICON_RSP);
			return;
		}
	}

	if (seq == 1) {
		fileInfo = std::make_shared<FileInfo>();
		fileInfo->_md5 = md5;
		fileInfo->_name = name;
		fileInfo->_seq = seq;
		fileInfo->_trans_size = trans_size;
		fileInfo->_total_size = total_size;
		auto targetDir = ConfigMgr::GetInstance()->getOutputPath();
		targetPath = (targetDir / name).string();
		fileInfo->_file_path = targetPath;
		LogicSystem::GetInstance()->AddFileInfo(md5, fileInfo);
		bool success = RedisMgr::GetInstance()->SetFileInfo(md5, fileInfo);
		if (!success) {
			rtvalue["error"] = ErrorCodes::SaveFileInfoFailed;
			session->Send(rtvalue.toStyledString(), MSG_UPLOAD_HEAD_ICON_RSP);
			return;
		}
	}
	else {
		auto fileInfo = LogicSystem::GetInstance()->GetFileInfo(md5);
		fileInfo->_seq = seq;
		fileInfo->_trans_size = trans_size;
		targetPath = fileInfo->_file_path;
		bool success = RedisMgr::GetInstance()->SetFileInfo(md5, fileInfo);
		if (!success) {
			rtvalue["error"] = ErrorCodes::SaveFileInfoFailed;
			session->Send(rtvalue.toStyledString(), MSG_UPLOAD_HEAD_ICON_RSP);
			return;
		}
	}

	auto callback = [=](const Json::Value& rtvalue) {
		Json::Value result = rtvalue;
		result["error"] = ErrorCodes::SUCCESS;
		result["md5"] = md5;
		result["name"] = name;
		result["seq"] = seq;
		result["trans_size"] = trans_size;
		result["total_size"] = total_size;
		result["finished"] = finished;
		session->Send(result.toStyledString(), MSG_UPLOAD_HEAD_ICON_RSP);
		};

	auto fileTask = std::make_shared<FileTask>(session, callback);
	fileTask->_name = name;
	fileTask->_seq = seq;
	fileTask->_file_path = targetPath;
	fileTask->_total_size = total_size;
	fileTask->_file_data = file_data;
	fileTask->_trans_size = trans_size;
	fileTask->_finished = finished;
	fileTask->_uid = uid;
	fileTask->_type = FileTaskType::UploadHeadIcon;

	std::hash<std::string> hash_str;
	auto value = hash_str(session->GetUuid());
	int index = value % FILE_WORKER_COUNT;
	FileSystem::GetInstance()->PostTask(fileTask, index);
}

void LogicWorker::HandleSyncUploadFile(std::shared_ptr<CSession> session, const short& id, const std::string& msg_data){
	/*Json::Value root, rtvalue;
	Json::Reader reader;
	reader.parse(msg_data, root);
	Defer defer([&rtvalue, session]() {
		session->Send(rtvalue.toStyledString(), MSG_SYNC_UPLOAD_FILE_RSP);
		});

	auto md5 = root["md5"].asString();

	auto fileInfo = LogicSystem::GetInstance()->GetFileInfo(md5);
	if (!fileInfo) {
		std::cout << "The file not exists" << std::endl;
		rtvalue["error"] = 1;
		return;
	}

	rtvalue["error"] = ErrorCodes::SUCCESS;
	rtvalue["md5"] = md5;
	rtvalue["name"] = fileInfo->_name;
	rtvalue["seq"] = fileInfo->_seq;
	rtvalue["trans_size"] = fileInfo->_trans_size;
	rtvalue["total_size"] = fileInfo->_total_size;*/
}

void LogicWorker::HandleDownLoadFile(std::shared_ptr<CSession> session, const short& id, const std::string& msg_data){
	Json::Value root;
	Json::Reader reader;
	reader.parse(msg_data, root);

	auto name = root["name"].asString();
	auto seq = root["seq"].asInt();
	auto trans_size = root["trans_size"].asInt();
	auto total_size = root["total_size"].asInt64();

	auto callback = [=](const Json::Value& rtvalue) {
		Json::Value result;
		result["error"] = rtvalue["error"].asInt();
		result["name"] = rtvalue["name"].asString();
		result["seq"] = rtvalue["seq"].asInt();
		result["trans_size"] = rtvalue["trans_size"].asInt();
		result["total_size"] = rtvalue["total_size"].asInt64();
		result["data"] = rtvalue["data"].asString();
		result["finished"] = rtvalue["finished"].asBool();
		session->Send(result.toStyledString(), MSG_DOWNLOAD_FILE_RSP);
		};

	auto task = std::make_shared<DownLoadTask>(session, callback);
	task->_name = name;
	task->_seq = seq;
	task->_trans_size = trans_size;
	task->_total_size = total_size;

	std::hash<std::string> hash_str;
	auto value = hash_str(session->GetUuid());
	short index = value % FILE_WORKER_COUNT;
	FileSystem::GetInstance()->PostDownLoadTask(task, index);
}

void LogicWorker::HandleUploadChatImgMsg(std::shared_ptr<CSession> session, const short& id, const std::string& msg_data){
	Json::Value root, rtvalue;
	Json::Reader reader;
	reader.parse(msg_data, root);

	auto md5 = root["md5"].asString();
	auto name = root["name"].asString();
	auto seq = root["seq"].asInt();
	auto trans_size = root["trans_size"].asInt();
	auto total_size = root["total_size"].asInt64();
	auto thread_id = root["thread_id"].asInt64();
	auto originalData = root["data"].asString();
	auto msg_id = root["message_id"].asInt64();
	auto uid = root["uid"].asInt();
	auto recv_uid = root["recv_uid"].asInt();
	auto token = root["token"].asString();
	auto finished = root["finished"].asBool();
	std::cout << "name: " << name << ", seq: " << seq << ", trans_size: " << trans_size << ", md5: " << md5 << ", finished: " << finished << std::endl;

	if (seq == 1) {
		std::string key = USERTOKENPREFIX + std::to_string(uid);
		std::string token_value;
		bool _b_get = RedisMgr::GetInstance()->Get(key, token_value);
		if (!_b_get) {
			std::cout << "Get token value failed, uid: " << uid << std::endl;
			rtvalue["error"] = ErrorCodes::ERR_REDIS;
			session->Send(rtvalue.toStyledString(), MSG_UPLOAD_IMG_CHAT_MSG_RSP);
			return;
		}

		if (token_value != token) {
			std::cout << "Token not match, Redis token: " << token_value << ", user token: " << token << std::endl;
			rtvalue["error"] = ErrorCodes::TokenInvalid;
			session->Send(rtvalue.toStyledString(), MSG_UPLOAD_IMG_CHAT_MSG_RSP);
			return;
		}
	}

	std::string base64Data = base64_decode(originalData);

	std::shared_ptr<FileInfo> fileInfo;
	if (seq == 1) {
		fileInfo = std::make_shared<FileInfo>();
		fileInfo->_md5 = md5;
		fileInfo->_name = name;
		fileInfo->_seq = seq;
		fileInfo->_trans_size = trans_size;
		fileInfo->_total_size = total_size;
		std::string dir_name = "thread_id_" + std::to_string(thread_id);
		auto targetDir = (ConfigMgr::GetInstance()->getOutputPath() / dir_name);
		// 没有则创建文件夹
		if (!std::filesystem::exists(targetDir)) {
			std::cout << "The dir not exists, dir: " << targetDir << ", will create dir" << std::endl;
			std::filesystem::create_directories(targetDir);
		}
		auto targetPath = (targetDir / name).string();
		fileInfo->_file_path = targetPath;
		LogicSystem::GetInstance()->AddFileInfo(md5, fileInfo);

		bool success = RedisMgr::GetInstance()->SetFileInfo(FILEUPLOADPREFIX + md5, fileInfo);
		if (!success) {
			std::cout << "Set Redis file info failed, file name: " << name << std::endl;
			rtvalue["error"] = ErrorCodes::ERR_REDIS;
			session->Send(rtvalue.toStyledString(), MSG_UPLOAD_IMG_CHAT_MSG_RSP);
			return;
		}
	}
	else {
		fileInfo = LogicSystem::GetInstance()->GetFileInfo(md5);
		if (!fileInfo) {
			std::cout << "The file info is nullptr, file md5: " << md5 << ", name: " << name << std::endl;
			rtvalue["error"] = ErrorCodes::FileNotExists;
			session->Send(rtvalue.toStyledString(), MSG_UPLOAD_IMG_CHAT_MSG_RSP);
			return;
		}

		fileInfo->_seq = seq;
		fileInfo->_trans_size = trans_size;

		bool success = RedisMgr::GetInstance()->SetFileInfo(FILEUPLOADPREFIX + md5, fileInfo);
		if (!success) {
			std::cout << "Set Redis file info failed, file name: " << name << std::endl;
			rtvalue["error"] = ErrorCodes::ERR_REDIS;
			session->Send(rtvalue.toStyledString(), MSG_UPLOAD_IMG_CHAT_MSG_RSP);
			return;
		}
	}

	auto callback = [=](const Json::Value& rtvalue) {
		Json::Value result;
		result["error"] = rtvalue["error"].asInt();
		result["md5"] = md5;
		result["name"] = name;
		result["seq"] = seq;
		result["trans_size"] = fileInfo->_trans_size;
		result["total_size"] = total_size;
		result["thread_id"] = thread_id;
		result["message_id"] = msg_id;
		result["finished"] = rtvalue["finished"].asBool();
		result["uid"] = uid;
		result["recv_uid"] = recv_uid;
		session->Send(result.toStyledString(), MSG_UPLOAD_IMG_CHAT_MSG_RSP);
		};

	std::hash<std::string> hash_str;
	auto value = hash_str(session->GetUuid());
	short index = value % FILE_WORKER_COUNT;

	auto task = std::make_shared<FileTask>(session, callback);
	task->_name = name;
	task->_seq = seq;
	task->_trans_size = fileInfo->_trans_size;
	task->_total_size = total_size;
	task->_file_path = fileInfo->_file_path;
	task->_file_data = base64Data;
	task->_task_callback = callback;
	task->_uid = uid;
	task->_finished = finished;
	task->_type = FileTaskType::UploadImgChatMsg;
	FileSystem::GetInstance()->PostTask(task, index);

	if (finished) {
		// Grpc通知ChatServer
		GetChatImgMsgRsp rsp = ResourceGrpcClient::GetInstance()->NotifyChatImgMsg(msg_id, uid, recv_uid, name, thread_id, md5, total_size);
		std::cout << "Grpc result: " << rsp.error() << std::endl;
	}
}

void LogicWorker::HandleDownloadChatImgMsg(std::shared_ptr<CSession> session, const short& id, const std::string& msg_data){
	Json::Value root;
	Json::Reader reader;
	reader.parse(msg_data, root);

	auto name = root["name"].asString();
	auto seq = root["seq"].asInt();
	auto trans_size = root["trans_size"].asInt();
	auto total_size = root["total_size"].asInt64();
	auto dir_name = root["dir_name"].asString();
	auto thread_id = root["thread_id"].asInt64();
	auto send_uid = root["send_uid"].asInt();
	bool finished = (static_cast<int64_t>(trans_size) >= total_size);

	auto callback = [=](const Json::Value& rtvalue) {
		Json::Value result;
		result["error"] = rtvalue["error"].asInt();
		result["name"] = name;
		result["seq"] = seq;
		result["trans_size"] = trans_size;
		result["total_size"] = total_size;
		result["data"] = rtvalue["data"].asString();
		result["dir_name"] = dir_name;
		result["thread_id"] = thread_id;
		result["send_uid"] = send_uid;
		result["finished"] = rtvalue["finished"].asBool();
		session->Send(result.toStyledString(), MSG_DOWNLOAD_IMG_CHAT_MSG_RSP);
		};

	auto task = std::make_shared<DownLoadTask>(session, callback);
	task->_name = name;
	task->_seq = seq;
	task->_trans_size = trans_size;
	task->_total_size = total_size;
	task->_dir_name = dir_name;
	
	std::hash<std::string> hash_str;
	auto value = hash_str(session->GetUuid());
	short index = value % FILE_WORKER_COUNT;
	FileSystem::GetInstance()->PostDownLoadTask(task, index);
}

void LogicWorker::HandleUploadChatVideoMsg(std::shared_ptr<CSession> session, const short& id, const std::string& data){
	Json::Value root, rtvalue;
	Json::Reader reader;
	reader.parse(data, root);

	auto token = root["token"].asString();
	auto seq = root["seq"].asInt();
	auto md5 = root["md5"].asString();
	auto name = root["name"].asString();
	auto uid = root["uid"].asInt();
	auto recv_uid = root["recv_uid"].asInt();
	auto thread_id = root["thread_id"].asInt64();
	auto trans_size = root["trans_size"].asInt();
	auto total_size = root["total_size"].asInt64();
	auto msg_id = root["message_id"].asInt64();
	auto originalData = root["data"].asString();
	auto type = root["type"].asString();
	auto duration = root["duration"].asInt();
	auto finished = root["finished"].asBool();

	if (seq == 1) {
		std::string key = USERTOKENPREFIX + std::to_string(uid);
		std::string token_value;
		bool _b_get = RedisMgr::GetInstance()->Get(key, token_value);
		if (!_b_get) {
			std::cout << "Get token value failed, uid: " << uid << std::endl;
			rtvalue["error"] = ErrorCodes::ERR_REDIS;
			session->Send(rtvalue.toStyledString(), MSG_UPLOAD_IMG_CHAT_MSG_RSP);
			return;
		}

		if (token_value != token) {
			std::cout << "Token not match, Redis token: " << token_value << ", user token: " << token << std::endl;
			rtvalue["error"] = ErrorCodes::TokenInvalid;
			session->Send(rtvalue.toStyledString(), MSG_UPLOAD_IMG_CHAT_MSG_RSP);
			return;
		}
	}

	std::string base64Data = base64_decode(originalData);

	std::shared_ptr<FileInfo> fileInfo;
	if (seq == 1) {
		fileInfo = std::make_shared<FileInfo>();
		fileInfo->_md5 = md5;
		fileInfo->_name = name;
		fileInfo->_seq = seq;
		fileInfo->_trans_size = trans_size;
		fileInfo->_total_size = total_size;
		std::string dir_name = (type == "cover") ? "video_covers" : "thread_id_" + std::to_string(thread_id);
		auto targetDir = (ConfigMgr::GetInstance()->getOutputPath() / dir_name);
		// 没有则创建文件夹
		if (!std::filesystem::exists(targetDir)) {
			std::cout << "The dir not exists, dir: " << targetDir << ", will create dir" << std::endl;
			std::filesystem::create_directories(targetDir);
		}
		auto targetPath = (targetDir / name).string();
		fileInfo->_file_path = targetPath;
		LogicSystem::GetInstance()->AddFileInfo(md5, fileInfo);

		bool success = RedisMgr::GetInstance()->SetFileInfo(FILEUPLOADPREFIX + md5, fileInfo);
		if (!success) {
			std::cout << "Set Redis file info failed, file name: " << name << std::endl;
			rtvalue["error"] = ErrorCodes::ERR_REDIS;
			session->Send(rtvalue.toStyledString(), MSG_UPLOAD_IMG_CHAT_MSG_RSP);
			return;
		}
	}
	else {
		fileInfo = LogicSystem::GetInstance()->GetFileInfo(md5);
		if (!fileInfo) {
			std::cout << "The file info is nullptr, file md5: " << md5 << ", name: " << name << std::endl;
			rtvalue["error"] = ErrorCodes::FileNotExists;
			session->Send(rtvalue.toStyledString(), MSG_UPLOAD_IMG_CHAT_MSG_RSP);
			return;
		}

		fileInfo->_seq = seq;
		fileInfo->_trans_size = trans_size;

		bool success = RedisMgr::GetInstance()->SetFileInfo(FILEUPLOADPREFIX + md5, fileInfo);
		if (!success) {
			std::cout << "Set Redis file info failed, file name: " << name << std::endl;
			rtvalue["error"] = ErrorCodes::ERR_REDIS;
			session->Send(rtvalue.toStyledString(), MSG_UPLOAD_IMG_CHAT_MSG_RSP);
			return;
		}
	}

	auto callback = [=](const Json::Value& rtvalue) {
		Json::Value result;
		result["error"] = rtvalue["error"].asInt();
		result["md5"] = md5;
		result["name"] = name;
		result["seq"] = seq;
		result["trans_size"] = fileInfo->_trans_size;
		result["total_size"] = total_size;
		result["thread_id"] = thread_id;
		result["message_id"] = msg_id;
		result["finished"] = rtvalue["finished"].asBool();
		result["uid"] = uid;
		result["recv_uid"] = recv_uid;
		session->Send(result.toStyledString(), MSG_UPLOAD_VIDEO_CHAT_MSG_RSP);
		};

	std::hash<std::string> hash_str;
	auto value = hash_str(session->GetUuid());
	short index = value % FILE_WORKER_COUNT;

	auto task = std::make_shared<FileTask>(session, callback);
	task->_name = name;
	task->_seq = seq;
	task->_trans_size = fileInfo->_trans_size;
	task->_total_size = total_size;
	task->_file_path = fileInfo->_file_path;
	task->_file_data = base64Data;
	task->_task_callback = callback;
	task->_uid = uid;
	task->_finished = finished;
	task->_type = (type == "cover") ? FileTaskType::UploadVideoCover : FileTaskType::UploadVideoBody;
	FileSystem::GetInstance()->PostTask(task, index);

	if (finished && type == "body") { 
		std::string coverName = name;
		std::string suffix = ".mp4";
		std::size_t pos = coverName.rfind(suffix);
		if (pos != std::string::npos && pos == coverName.length() - suffix.length()) {
			coverName.replace(pos, suffix.length(), "_cover.jpg");
		}

		// Grpc通知ChatServer
		GetChatVideoMsgRsp rsp = ResourceGrpcClient::GetInstance()->NotifyChatVideoMsg(msg_id, uid, recv_uid, name, coverName, thread_id, duration);
		std::cout << "Grpc result: " << rsp.error() << std::endl;
	}
}

void LogicWorker::HandleDownloadChatVideoCover(std::shared_ptr<CSession> session, const short& id, const std::string& data){
	Json::Value root;
	Json::Reader reader;
	reader.parse(data, root);

	auto name = root["name"].asString();
	auto seq = root["seq"].asInt();
	auto trans_size = root["trans_size"].asInt();
	auto dir_name = root["dir_name"].asString();
	auto thread_id = root["thread_id"].asInt64();
	auto send_uid = root["send_uid"].asInt();

	auto callback = [=](const Json::Value& rtvalue) {
		Json::Value result;
		result["error"] = rtvalue["error"].asInt();
		result["name"] = name;
		result["seq"] = seq;
		result["trans_size"] = trans_size;
		result["total_size"] = rtvalue["total_size"].asInt64();
		result["data"] = rtvalue["data"].asString();
		result["dir_name"] = dir_name;
		result["thread_id"] = thread_id;
		result["send_uid"] = send_uid;
		result["finished"] = rtvalue["finished"].asBool();
		session->Send(result.toStyledString(), MSG_DOWNLOAD_VIDEO_COVER_CHAT_MSG_RSP);
		};

	auto task = std::make_shared<DownLoadTask>(session, callback);
	task->_name = name;
	task->_seq = seq;
	task->_trans_size = trans_size;
	task->_dir_name = dir_name;

	std::hash<std::string> hash_str;
	auto value = hash_str(session->GetUuid());
	short index = value % FILE_WORKER_COUNT;
	FileSystem::GetInstance()->PostDownLoadTask(task, index);
}

void LogicWorker::HandleDownloadChatVideoBody(std::shared_ptr<CSession> session, const short& id, const std::string& data){
	Json::Value root;
	Json::Reader reader;
	reader.parse(data, root);

	auto name = root["name"].asString();
	auto seq = root["seq"].asInt();
	auto trans_size = root["trans_size"].asInt();
	auto dir_name = root["dir_name"].asString();
	auto thread_id = root["thread_id"].asInt64();
	auto send_uid = root["send_uid"].asInt();

	auto callback = [=](const Json::Value& rtvalue) {
		Json::Value result;
		result["error"] = rtvalue["error"].asInt();
		result["name"] = name;
		result["seq"] = seq;
		result["trans_size"] = trans_size;
		result["total_size"] = rtvalue["total_size"].asInt64();
		result["data"] = rtvalue["data"].asString();
		result["dir_name"] = dir_name;
		result["thread_id"] = thread_id;
		result["send_uid"] = send_uid;
		result["finished"] = rtvalue["finished"].asBool();
		session->Send(result.toStyledString(), MSG_DOWNLOAD_VIDEO_BODY_CHAT_MSG_RSP);
		};

	auto task = std::make_shared<DownLoadTask>(session, callback);
	task->_name = name;
	task->_seq = seq;
	task->_trans_size = trans_size;
	task->_dir_name = dir_name;

	std::hash<std::string> hash_str;
	auto value = hash_str(session->GetUuid());
	short index = value % FILE_WORKER_COUNT;
	FileSystem::GetInstance()->PostDownLoadTask(task, index);
}
