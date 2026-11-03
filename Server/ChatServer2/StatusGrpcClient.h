#pragma once
#include "Singleton.h"
#include "StatusConPool.h"

class StatusGrpcClient : public Singleton<StatusGrpcClient>{
	friend class Singleton<StatusGrpcClient>;
public:
	~StatusGrpcClient();
	GetChatServerRsp GetChatServer(int uid);
	LoginRsp Login(int uid, const std::string& token);
private:
	StatusGrpcClient();
	std::unique_ptr<StatusConPool> _pool;
};

