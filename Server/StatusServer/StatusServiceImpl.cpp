#include "StatusServiceImpl.h"
#include "ConfigMgr.h"
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "Global.h"
#include <vector>
#include <sstream>
#include <string>
#include "RedisMgr.h"

std::string generate_random_uuid() {
	return boost::uuids::to_string(boost::uuids::random_generator()());
}

StatusServiceImpl::StatusServiceImpl() {
	auto config = ConfigMgr::GetInstance();
	std::string server_list = (*config)["ChatServers"]["Name"];

	std::stringstream ss(server_list);
	std::vector<std::string> words;
	std::string word;
	while (std::getline(ss, word, ',')){
		words.push_back(word);
	}

	for (const auto& name : words) {
		if ((*config)[name]["Name"].empty()) {
			continue;
		}

		ChatServer server;
		server._name = (*config)[name]["Name"];
		server._host = (*config)[name]["Host"];
		server._port = (*config)[name]["Port"];
		
		std::string login_count = RedisMgr::GetInstance()->HGet(LOGIN_COUNT, server._name);
		std::cout << "server name: " << server._name << ", login count: " << login_count << std::endl;
		if (!login_count.empty()) {
			server._con_count = std::atoi(login_count.c_str());
		}
		_servers[server._name] = server;
	}

	std::string _res_host = (*config)["ResourceServer"]["Host"];
	std::string _res_port = (*config)["ResourceServer"]["Port"];
	_res_server = ResourceServer(_res_host, _res_port);
}

::grpc::Status StatusServiceImpl::GetChatServer(::grpc::ServerContext* context, const::message::GetChatServerReq* request, ::message::GetChatServerRsp* reply){
	std::string prefix = "wechat status sever has received: ";
	const auto& server = getChatServer();
	reply->set_host(server._host);
	reply->set_port(server._port);
	reply->set_error(ErrorCodes::SUCCESS);
	reply->set_token(generate_random_uuid());
	insertToken(std::to_string(request->uid()), reply->token());
	return Status::OK;
}

::grpc::Status StatusServiceImpl::GetResourceServer(::grpc::ServerContext* context, const ::message::GetResourceServerReq* req, ::message::GetResourceServerRsp* rsp) {
	rsp->set_error(ErrorCodes::SUCCESS);
	rsp->set_host(_res_server._host);
	rsp->set_port(_res_server._port);
	return Status::OK;
}

::grpc::Status StatusServiceImpl::Login(::grpc::ServerContext* context, const::message::LoginReq* request, ::message::LoginRsp* reply){
	auto uid = request->uid();
	auto token = request->token();

	std::string token_key = USERTOKENPREFIX + std::to_string(uid);
	std::string redisToken;
	bool result = RedisMgr::GetInstance()->Get(token_key, redisToken);
	if (!result) {
		std::cout << "Get value failed" << std::endl;
		reply->set_error(ErrorCodes::UidInvalid);
		return Status::OK;
	}

	if (token != redisToken) {
		std::cout << "token not match" << std::endl;
		reply->set_error(ErrorCodes::TokenInvalid);
		return Status::OK;
	}

	reply->set_error(ErrorCodes::SUCCESS);
	reply->set_uid(uid);
	reply->set_token(redisToken);
	return Status::OK;
}

void StatusServiceImpl::insertToken(std::string uid, std::string token){
	std::string token_key = USERTOKENPREFIX + uid;
	RedisMgr::GetInstance()->Set(token_key, token);
}

ChatServer StatusServiceImpl::getChatServer(){
	std::unique_lock<std::mutex> lock(_mtx);
	auto identifier = RedisMgr::GetInstance()->acquireLock(LOCK_TIME_OUT, ACQUIRE_TIME_OUT);
	Defer defer([&identifier]() {
		RedisMgr::GetInstance()->releaseLock(identifier);
		});

	ChatServer minServer = _servers.begin()->second;
	for (auto& server : _servers) {
		std::string login_count = RedisMgr::GetInstance()->HGet(LOGIN_COUNT, server.second._name);
		if (!login_count.empty()) {
			server.second._con_count = std::atoi(login_count.c_str());
		}
		else {
			server.second._con_count = INT_MAX;
		}

		if (minServer._con_count > server.second._con_count) {
			minServer = server.second;
		}
	}
	return minServer;
}

ChatServer::ChatServer() : _name(""), _host(""), _port(""), _con_count(0){}

ChatServer::ChatServer(const ChatServer& other) : _name(other._name), _host(other._host), _port(other._port), _con_count(other._con_count){}

ChatServer& ChatServer::operator=(const ChatServer& other){
	if (this == &other) {
		return *this;
	}

	_name = other._name;
	_host = other._host;
	_port = other._port;
	_con_count = other._con_count;
	return *this;
}

ResourceServer::ResourceServer(const std::string& host, const std::string& port): _host(host), _port(port)
{}
