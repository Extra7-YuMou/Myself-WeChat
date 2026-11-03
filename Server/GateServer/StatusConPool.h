#pragma once
#include "Singleton.h"
#include "message.grpc.pb.h"
#include "message.pb.h"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <grpcpp/grpcpp.h>

using grpc::Status;
using grpc::ClientContext;
using grpc::Channel;

using message::StatusService;
using message::GetChatServerReq;
using message::GetChatServerRsp;
using message::GetResourceServerReq;
using message::GetResourceServerRsp;

class StatusConPool{
public:
	StatusConPool(std::size_t size, const std::string& host, const std::string& port);
	std::unique_ptr<StatusService::Stub> GetConnection();
	void returnConnection(std::unique_ptr < StatusService::Stub> connection);
	~StatusConPool();
	void Close();
private:
	std::size_t _poolSize;
	std::string _host;
	std::string _port;
	std::queue <std::unique_ptr< StatusService::Stub >> _stubs;
	std::mutex _mtx;
	std::condition_variable _cond;
	std::atomic<bool> _b_stop;
};

