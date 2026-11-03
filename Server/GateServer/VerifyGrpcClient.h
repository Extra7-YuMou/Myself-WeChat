#pragma once
#include <grpcpp/grpcpp.h>
#include "Singleton.h"
#include "message.grpc.pb.h"
#include "message.pb.h"

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::VerifyService;
using message::GetVerifyReq;
using message::GetVerifyRsp;

class VerifyGrpcClient : public Singleton<VerifyGrpcClient>{
	friend class Singleton<VerifyGrpcClient>;
public:
	~VerifyGrpcClient();

	GetVerifyRsp GetVerifyCode(std::string email);

private:
	VerifyGrpcClient();

	std::unique_ptr<VerifyService::Stub> stub_;
};

