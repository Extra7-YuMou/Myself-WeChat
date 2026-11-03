#pragma once
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include "message.pb.h"

using grpc::Status;
using grpc::ServerContext;
using grpc::Channel;

using message::StatusService;
using message::GetChatServerReq;
using message::GetChatServerRsp;

struct ChatServer {
	ChatServer();
	ChatServer(const ChatServer& other);
	ChatServer& operator=(const ChatServer& other);

	std::string _name;
	std::string _host;
	std::string _port;
	int _con_count;
};

struct ResourceServer {
	ResourceServer(): _host(""), _port(""){}
	ResourceServer(const std::string&, const std::string&);
	std::string _host;
	std::string _port;
};

class StatusServiceImpl final : public StatusService::Service{
public:
	StatusServiceImpl();
	virtual ::grpc::Status GetChatServer(::grpc::ServerContext* context, const ::message::GetChatServerReq* request, ::message::GetChatServerRsp* response) override;
	virtual ::grpc::Status GetResourceServer(::grpc::ServerContext* context, const ::message::GetResourceServerReq* request, ::message::GetResourceServerRsp* response);
	virtual ::grpc::Status Login(::grpc::ServerContext* context, const ::message::LoginReq* request, ::message::LoginRsp* response) override;
private:
	void insertToken(std::string uid, std::string token);
	ChatServer getChatServer();
	std::unordered_map<std::string, ChatServer> _servers;
	std::mutex _mtx;
	ResourceServer _res_server;
};

