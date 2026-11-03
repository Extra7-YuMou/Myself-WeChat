#pragma once
#include "Global.h"
#include "MsgNode.h"

class CServer;
class CSession : public std::enable_shared_from_this<CSession>{
public:
	CSession(boost::asio::io_context& ioc, CServer* server);
	void start();
	void Send(const std::string& msg, short id);
	tcp::socket& getSocket() { return _sock; }
	std::string getUuid() { return _uuid; }
	void setUid(int uid) { _uid = uid; }
	int getUid() { return _uid; }
	void close();
	void notifyOffline(int uid);
	CServer* getServer();
	void DealExceptionSession();
	bool IsHeartBeatExpried(std::time_t& now);
	void UpdateHeartBeat();
private:
	void handleRead(boost::system::error_code ec, std::size_t bytes_transferred, std::shared_ptr<CSession> _self);
	void handleWrite(boost::system::error_code ec, std::size_t bytes_transferred, std::shared_ptr<CSession> _self);

	CServer* _server;
	tcp::socket _sock;
	std::queue<std::shared_ptr<SendNode>> _send_que;
	std::shared_ptr<MsgNode> _recv_head_node;
	std::shared_ptr<RecvNode> _recv_msg_node;
	char _data[MAX_LENGTH];
	std::string _uuid;
	bool _b_recv_head;
	std::mutex _mtx;
	int _uid;
	std::time_t _last_heartbeat;
};

class LogicNode {
public:
	LogicNode(std::shared_ptr<CSession> session, std::shared_ptr<RecvNode> recvNode) : _session(session), _recv_node(recvNode){}
	std::shared_ptr<CSession> _session;
	std::shared_ptr<RecvNode> _recv_node;
};