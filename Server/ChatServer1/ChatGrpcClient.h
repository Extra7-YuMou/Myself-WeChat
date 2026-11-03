#pragma once
#include "ChatConPool.h"
#include <unordered_map>
#include <string>
#include "data.h"
#include <json/value.h>

class ChatGrpcClient : public Singleton<ChatGrpcClient>{
	friend class Singleton<ChatGrpcClient>;
public:
	~ChatGrpcClient();
	AddFriendRsp NotifyAddFriend(std::string server_ip, const AddFriendReq& req);
	AuthFriendRsp NotifyAuthFriend(std::string server_ip, const AuthFriendReq& req);
	TextChatMsgRsp NotifyTextChatMsg(std::string server_ip, const TextChatMsgReq& req, const Json::Value& rtvalue);
	KickUserRsp NotifyKickUser(const std::string& server_ip, int uid);
	ImgChatMsgRsp NotifyImgChatMsg(const std::string& server_ip, const Json::Value& rtvalue);
private:
	ChatGrpcClient();

	std::unordered_map<std::string, std::unique_ptr<ChatConPool>> _pools;
	std::string RpcPort;
};

