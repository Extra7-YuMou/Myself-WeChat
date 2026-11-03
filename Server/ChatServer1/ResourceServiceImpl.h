#pragma once
#include "message.grpc.pb.h"
#include <grpcpp/grpcpp.h>
#include <memory>
#include <json/json.h>
#include <json/reader.h>
#include <json/value.h>
#include "CSession.h"

using grpc::ServerContext;
using grpc::Server;
using grpc::Channel;
using grpc::Status;

using message::GetChatImgMsgReq;
using message::GetChatImgMsgRsp;
using message::ResourceService;

using message::GetChatVideoMsgReq;
using message::GetChatVideoMsgRsp;

class ResourceServiceImpl final : public ResourceService::Service{
public:
	ResourceServiceImpl() = default;
	Status NotifyChatImgMsg(::grpc::ServerContext* context, const ::message::GetChatImgMsgReq* request, ::message::GetChatImgMsgRsp* response);
	Status NotifyChatVideoMsg(::grpc::ServerContext* context, const ::message::GetChatVideoMsgReq* request, ::message::GetChatVideoMsgRsp* response);
};

