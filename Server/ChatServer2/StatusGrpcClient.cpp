#include "StatusGrpcClient.h"
#include "ConfigMgr.h"
#include "Global.h"

StatusGrpcClient::StatusGrpcClient() {
	auto config = ConfigMgr::GetInstance();
	std::string host = (*config)["StatusServer"]["Host"];
	std::string port = (*config)["StatusServer"]["Port"];
	_pool = std::make_unique<StatusConPool>(5, host, port);
}

StatusGrpcClient::~StatusGrpcClient(){
	_pool->Close();
}

GetChatServerRsp StatusGrpcClient::GetChatServer(int uid){
	ClientContext context;
	GetChatServerReq req;
	GetChatServerRsp reply;
	req.set_uid(uid);
	auto stub = _pool->getStub();
	
	Defer defer([this, &stub]() {
		_pool->returnStub(std::move(stub));
		});

	Status status = stub->GetChatServer(&context, req, &reply);
	if (status.ok()) {
		return reply;
	}
	else {
		reply.set_error(ErrorCodes::ERR_RPCFialed);
		return reply;
	}
}

LoginRsp StatusGrpcClient::Login(int uid, const std::string& token){
	ClientContext context;
	LoginReq req;
	LoginRsp reply;
	req.set_uid(uid);
	req.set_token(token);
	auto stub = _pool->getStub();

	Defer defer([this, &stub]() {
		_pool->returnStub(std::move(stub));
		});

	Status status = stub->Login(&context, req, &reply);
	if (status.ok()) {
		return reply;
	}
	else {
		reply.set_error(ErrorCodes::ERR_RPCFialed);
		return reply;
	}
}
