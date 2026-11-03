#pragma once
#include "message.grpc.pb.h"
#include "message.pb.h"
#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <grpcpp/grpcpp.h>

using message::GetChatImgMsgReq;
using message::GetChatImgMsgRsp;
using message::ResourceService;

using grpc::Server;
using grpc::ClientContext;
using grpc::Status;
using grpc::Channel;

class ResourceConPool{
public:
	ResourceConPool(const std::string& _host, const std::string& port, const std::size_t& size);
	void Close();
	~ResourceConPool();
	std::unique_ptr<ResourceService::Stub> GetStub();
	void ReturnStub(std::unique_ptr<ResourceService::Stub> stub);
private:
	std::queue<std::unique_ptr<ResourceService::Stub>> _stubs;
	std::atomic<bool> _b_stop;
	std::mutex _mtx;
	std::condition_variable _cond;
	std::string _host;
	std::string _port;
	std::size_t _size;
};

