#pragma once
#include <iostream>
#include <boost/asio.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "MsgNode.h"
#include <queue>
#include "Global.h"

using tcp = boost::asio::ip::tcp;

class CServer;
class CSession: public std::enable_shared_from_this<CSession>{
public:
	CSession(boost::asio::io_context& ioc, CServer* server);
	void Start();
	void Send(const std::string& msg, short id);
	tcp::socket& GetSocket() { return _sock; }
	std::string GetUuid() { return _uuid; }
private:
	void HandleRead(boost::system::error_code ec, std::size_t bytes_transferred, std::shared_ptr<CSession>);
	void AsyncReadHead();
	void AsyncReadBody(std::size_t total_len);
	void AsyncReadFull(std::size_t total_len, std::function<void(const boost::system::error_code, std::size_t bytes_transferred)> handler);
	void AsyncReadLen(std::size_t read_len, std::size_t total_len, 
		std::function<void(const boost::system::error_code, std::size_t bytes_transferred)> handler);
	void HandleWrite(boost::system::error_code ec, std::size_t bytes_transferred, std::shared_ptr<CSession>);

	CServer* _server;
	tcp::socket _sock;
	std::shared_ptr<MsgNode> _recv_head_node;
	std::shared_ptr<RecvNode> _recv_msg_node;
	std::queue<std::shared_ptr<SendNode>> _send_que;
	std::atomic<bool> _b_recv_head;
	std::mutex _mtx;
	std::string _uuid;
	int _uid;
	char _buffer[MAX_BUFFER_LEN];
};

class LogicNode {
public:
	LogicNode(std::shared_ptr<CSession> session, std::shared_ptr<RecvNode> recv_node): _session(session), _recv_node(recv_node){}
	std::shared_ptr<CSession> _session;
	std::shared_ptr<RecvNode> _recv_node;
};