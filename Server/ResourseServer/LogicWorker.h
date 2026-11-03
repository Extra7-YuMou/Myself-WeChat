#pragma once
#include <iostream>
#include <string>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <map>
#include "CSession.h"
#include <json/json.h>
#include <json/reader.h>
#include <json/value.h>

class LogicWorker{
	using handle = std::function<void(std::shared_ptr<CSession>, const short&, const std::string&)>;
public:
	LogicWorker();
	void PostTask(std::shared_ptr<LogicNode>);
	~LogicWorker();
private:
	void InitHandlers();
	void TaskCallBack(std::shared_ptr<LogicNode>);
	void HandleUploadHeadIcon(std::shared_ptr<CSession>, const short&, const std::string&);
	void HandleSyncUploadFile(std::shared_ptr<CSession>, const short&, const std::string&);
	void HandleDownLoadFile(std::shared_ptr<CSession>, const short&, const std::string&);
	void HandleUploadChatImgMsg(std::shared_ptr<CSession>, const short&, const std::string&);
	void HandleDownloadChatImgMsg(std::shared_ptr<CSession>, const short&, const std::string&);
	void HandleUploadChatVideoMsg(std::shared_ptr<CSession>, const short&, const std::string&);
	void HandleDownloadChatVideoCover(std::shared_ptr<CSession>, const short&, const std::string&);
	void HandleDownloadChatVideoBody(std::shared_ptr<CSession>, const short&, const std::string&);

	std::map<short, handle> _handlers;
	std::queue<std::shared_ptr<LogicNode>> _logic_que;
	std::mutex _mtx;
	std::condition_variable _cond;
	std::thread _worker_thread;
	std::atomic<bool> _b_stop;
	std::mutex _file_mtx;
};

