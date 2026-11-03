#pragma once
#include <grpcpp/grpcpp.h>
#include <json/value.h>
#include <json/reader.h>
#include <json/json.h>
#include "message.grpc.pb.h"
#include "data.h"

using grpc::Channel;
using grpc::ServerContext;
using grpc::Status;

using message::AddFriendReq;
using message::AddFriendRsp;

using message::AuthFriendReq;
using message::AuthFriendRsp;

using message::GetChatServerRsp;
using message::LoginReq;
using message::LoginRsp;
using message::ChatService;

using message::TextChatData;
using message::TextChatMsgReq;
using message::TextChatMsgRsp;

class ChatServiceImpl final : public ChatService::Service {
public:
	ChatServiceImpl();
	Status NotifyAddFriend(::grpc::ServerContext* context, const ::message::AddFriendReq* request, ::message::AddFriendRsp* response);
	Status NotifyAuthFriend(::grpc::ServerContext* context, const ::message::AuthFriendReq* request, ::message::AuthFriendRsp* response);
	Status NotifyTextChatMsg(::grpc::ServerContext* context, const ::message::TextChatMsgReq* request, ::message::TextChatMsgRsp* response);
	Status NotifyKickUser(::grpc::ServerContext* context, const ::message::KickUserReq* request, ::message::KickUserRsp* response);
private:

};

