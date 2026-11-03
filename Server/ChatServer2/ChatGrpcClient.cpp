#include "ChatGrpcClient.h"
#include <sstream>
#include "ChatServiceImpl.h"
#include "Global.h"

ChatGrpcClient::~ChatGrpcClient(){
	for (auto& it : _pools) {
		it.second->close();
	}
}

AddFriendRsp ChatGrpcClient::NotifyAddFriend(std::string server_ip, const AddFriendReq& req)
{
	AddFriendRsp rsp;
	Defer deer([&rsp, &req]() {
		rsp.set_error(ErrorCodes::SUCCESS);
		rsp.set_fromuid(req.fromuid());
		rsp.set_touid(req.touid());
		});

	// 先找到对应服务器的连接池
	auto iter_pool = _pools.find(server_ip);
	if (iter_pool == _pools.end()) {
		return rsp;
	}

	// 获取Stub发送到对应服务器
	auto& pool = iter_pool->second;
	auto stub = pool->getStub();
	ClientContext context;
	Status status = stub->NotifyAddFriend(&context, req, &rsp);
	if (!status.ok()) {
		rsp.set_error(ErrorCodes::ERR_RPCFialed);
	}

	Defer defer_([&pool, &stub]() {
		pool->returnStub(std::move(stub));
		});
	return rsp;
}

AuthFriendRsp ChatGrpcClient::NotifyAuthFriend(std::string server_ip, const AuthFriendReq& req){
	AuthFriendRsp rsp;
	Defer defer([this, &rsp, &req]() {
		rsp.set_error(ErrorCodes::SUCCESS);
		rsp.set_fromuid(req.fromuid());
		rsp.set_touid(req.touid());
		});

	// 先找到对应服务器的连接
	auto iter_pool = _pools.find(server_ip);
	if (iter_pool == _pools.end()) {
		return rsp;
	}

	// 从连接池获取Stub发送
	auto stub = iter_pool->second->getStub();
	ClientContext context;
	Status status = stub->NotifyAuthFriend(&context, req, &rsp);
	if (!status.ok()) {
		rsp.set_error(ErrorCodes::ERR_RPCFialed);
	}

	Defer _defer([this, &stub, &iter_pool]() {
		iter_pool->second->returnStub(std::move(stub));
		});

	return rsp;
}

TextChatMsgRsp ChatGrpcClient::NotifyTextChatMsg(std::string server_ip, const TextChatMsgReq& req, const Json::Value& rtvalue){
	TextChatMsgRsp reply;
	reply.set_error(ErrorCodes::SUCCESS);
	Defer defer([&reply, &req, &rtvalue]() {
		reply.set_fromuid(req.fromuid());
		reply.set_touid(req.touid());
		for (const auto& obj : rtvalue["text_array"]) {
			auto content = obj["content"].asString();
			auto id = obj["msgid"].asString();
			auto* add_msg_info = reply.add_textmsgs();
			add_msg_info->set_msgid(id);
			add_msg_info->set_msgcontent(content);
		}
		});

	// 获取Stub
	auto iter_pool = _pools.find(server_ip);
	if (iter_pool == _pools.end()) {
		return reply;
	}

	auto& pool = iter_pool->second;
	ClientContext context;
	auto stub = pool->getStub();
	Defer _defer([&stub, &pool]() {
		pool->returnStub(std::move(stub));
		});

	Status status = stub->NotifyTextChatMsg(&context, req, &reply);
	if (!status.ok()) {
		reply.set_error(ErrorCodes::ERR_RPCFialed);
		return reply;
	}
	return reply;
}

KickUserRsp ChatGrpcClient::NotifyKickUser(const std::string& server_ip, int uid){
	ClientContext context;
	KickUserReq req;
	KickUserRsp rsp;
	req.set_uid(uid);

	Defer defer([&rsp, &uid]() {
		rsp.set_uid(uid);
		});

	auto iter_find = _pools.find(server_ip);
	if (iter_find == _pools.end()) {
		rsp.set_error(ErrorCodes::ERR_RPCFialed);
		return rsp;
	}

	auto& pool = iter_find->second;
	auto stub = pool->getStub();
	Defer defer2([&stub, &pool]() {
		pool->returnStub(std::move(stub));
		});

	Status status = stub->NotifyKickUser(&context, req, &rsp);
	if (!status.ok()) {
		rsp.set_error(ErrorCodes::ERR_RPCFialed);
	}
	return rsp;
}

ChatGrpcClient::ChatGrpcClient(){
	auto config = ConfigMgr::GetInstance();
	std::string word = (*config)["PeerServer"]["Servers"];

	std::vector<std::string> _words;
	std::stringstream ss(word);
	std::string line;

	while (std::getline(ss, line, ',')) {
		_words.push_back(line);
	}

	for (auto& name : _words) {
		if ((*config)[name]["Name"].empty()) {
			continue;
		}
		_pools[name] = std::make_unique<ChatConPool>(5, (*config)[name]["Host"], (*config)[name]["RPCPort"]);
	}
}
