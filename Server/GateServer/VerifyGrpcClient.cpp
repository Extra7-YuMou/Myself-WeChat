#include "VerifyGrpcClient.h"
#include "Global.h"

VerifyGrpcClient::~VerifyGrpcClient(){}

GetVerifyRsp VerifyGrpcClient::GetVerifyCode(std::string email)
{
	GetVerifyReq request;
	GetVerifyRsp reply;
	ClientContext context;
	request.set_email(email);

	grpc::Status status = stub_->GetVerifyCode(&context, request, &reply);
	if (status.ok()) {
		return reply;
	}
	else {
		reply.set_error(ErrorCodes::ERR_RPCFialed);
		return reply;
	}
}

VerifyGrpcClient::VerifyGrpcClient() {
	std::shared_ptr<Channel> channel = grpc::CreateChannel("127.0.0.1:50051", grpc::InsecureChannelCredentials());
	stub_ = std::make_unique<VerifyService::Stub>(channel);
}