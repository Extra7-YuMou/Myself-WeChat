#pragma once
#include "message.grpc.pb.h"
#include <grpcpp/grpcpp.h>
#include "ConfigMgr.h"
#include "Singleton.h"
#include <mutex>
#include <condition_variable>
#include <queue>

using grpc::ClientContext;
using grpc::Status;
using grpc::Channel;

using message::AddFriendReq;
using message::AddFriendRsp;

using message::AuthFriendReq;
using message::AuthFriendRsp;

using message::GetChatServerRsp;
using message::ChatService;
using message::LoginReq;
using message::LoginRsp;

using message::TextChatData;
using message::TextChatMsgReq;
using message::TextChatMsgRsp;

using message::KickUserReq;
using message::KickUserRsp;

using message::ImgChatData;
using message::ImgChatMsgReq;
using message::ImgChatMsgRsp;

class ChatConPool{
public:
	ChatConPool(std::size_t size, const std::string& host, const std::string& port);
	std::unique_ptr<ChatService::Stub> getStub();
	void returnStub(std::unique_ptr<ChatService::Stub> stub);
	void close();
	~ChatConPool();
private:
	std::atomic<bool> _b_stop;
	std::queue<std::unique_ptr<ChatService::Stub>> _stubs;
	std::size_t _size;
	std::string _host;
	std::string _port;
	std::mutex _mtx;
	std::condition_variable _cond;
};

