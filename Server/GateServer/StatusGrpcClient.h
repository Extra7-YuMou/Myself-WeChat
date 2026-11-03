#pragma once
#include "StatusConPool.h"

class StatusGrpcClient : public Singleton<StatusGrpcClient>{
	friend class Singleton<StatusGrpcClient>;
public:
	~StatusGrpcClient();
	GetChatServerRsp GetChatServer(int uid);
	GetResourceServerRsp GetResourceServer();
private:
	StatusGrpcClient();

	std::unique_ptr<StatusConPool> _pool;
};

