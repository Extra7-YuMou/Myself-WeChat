#include "StatusGrpcClient.h"
#include "ConfigMgr.h"
#include "Global.h"

StatusGrpcClient::~StatusGrpcClient(){}

GetChatServerRsp StatusGrpcClient::GetChatServer(int uid){
	ClientContext context;
	GetChatServerReq request;
	GetChatServerRsp reply;
	request.set_uid(uid);

	auto stub = _pool->GetConnection();
	Status status = stub->GetChatServer(&context, request, &reply);
	Defer defer([&stub, this]() {
		_pool->returnConnection(std::move(stub));
		});
	if (status.ok()) {
		return reply;
	}
	else {
		reply.set_error(ErrorCodes::ERR_RPCFialed);
		return reply;
	}
}

GetResourceServerRsp StatusGrpcClient::GetResourceServer(){
	GetResourceServerRsp rsp;
	GetResourceServerReq req;
	ClientContext context;

	auto stub = _pool->GetConnection();
	Defer defer([this, &stub]() {
		_pool->returnConnection(std::move(stub));
		});

	Status status = stub->GetResourceServer(&context, req, &rsp);
	if (!status.ok()) {
		rsp.set_error(ErrorCodes::ERR_RPCFialed);
	}
	rsp.set_error(ErrorCodes::SUCCESS);
	return rsp;
}

StatusGrpcClient::StatusGrpcClient() {
	auto config = ConfigMgr::GetInstance();
	std::string host = (*config)["StatusServer"]["Host"];
	std::string port = (*config)["StatusServer"]["Port"];
	_pool = std::make_unique<StatusConPool>(5, host, port);
}