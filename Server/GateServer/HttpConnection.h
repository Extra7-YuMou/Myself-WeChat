#pragma once
#include "Global.h"
#include "LogicSystem.h"

class HttpConnection : public std::enable_shared_from_this<HttpConnection>{
	friend class LogicSystem;
public:
	HttpConnection(boost::asio::io_context& ioc);
	void Start();
	tcp::socket& GetSocket() { return _sock; }
private:
	void CheckDeadline();
	void HandleReq();
	void WriteResponse();
	void PreParseGetParam();

	tcp::socket _sock;
	http::request<http::dynamic_body> _request;
	http::response<http::dynamic_body> _response;
	beast::flat_buffer _buffer{ 8192 };
	net::steady_timer _deadline{
		_sock.get_executor(), std::chrono::seconds(60)};
	std::string _get_url;
	std::unordered_map<std::string, std::string> _get_params;
};

