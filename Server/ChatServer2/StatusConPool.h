#pragma once
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include <mutex>
#include <condition_variable>
#include <memory>
#include <queue>

using grpc::ClientContext;
using grpc::Status;
using grpc::Server;

using message::StatusService;
using message::GetChatServerReq;
using message::GetChatServerRsp;
using message::LoginReq;
using message::LoginRsp;

class StatusConPool{
public:
	StatusConPool(std::size_t size, const std::string& host, const std::string& port);
	std::unique_ptr<StatusService::Stub> getStub();
	void returnStub(std::unique_ptr<StatusService::Stub> stub);
	void Close();
	~StatusConPool();
private:
	std::queue<std::unique_ptr<StatusService::Stub>> _stubs;
	std::size_t _size;
	std::string _host;
	std::string _port;
	std::atomic<bool> _b_stop;
	std::mutex _mtx;
	std::condition_variable _cond;
};

