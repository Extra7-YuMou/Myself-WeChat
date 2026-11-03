#pragma once
#include "message.grpc.pb.h"
#include "message.pb.h"
#include <memory>
#include "Singleton.h"
#include "ResourceConPool.h"

using message::GetChatImgMsgReq;
using message::GetChatImgMsgRsp;
using message::ResourceService;

using message::GetChatVideoMsgReq;
using message::GetChatVideoMsgRsp;

using grpc::Status;
using grpc::Server;
using grpc::ClientContext;

class ResourceGrpcClient : public Singleton<ResourceGrpcClient>{
	friend class Singleton<ResourceGrpcClient>;
public:
	GetChatImgMsgRsp NotifyChatImgMsg(const int64_t& msg_id, const int& uid, const int&recv_uid, 
		const std::string& name, const int64_t& thread_id, const std::string& md5, const int64_t& file_size);
	GetChatVideoMsgRsp NotifyChatVideoMsg(const int64_t& msg_id, const int& uid, const int& recv_uid,
		const std::string& name, const std::string& cover_name, const int64_t& thread_id, const int& duration);
private:
	ResourceGrpcClient();
	std::unique_ptr<ResourceConPool> _pool;
};

