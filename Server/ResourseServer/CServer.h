#pragma once
#include "CSession.h"
#include <map>

class CServer{
public:
	CServer(boost::asio::io_context& ioc, unsigned short port);
	void Start();
	void ClearSession(const std::string& uuid);
private:
	void HandleAccept(std::shared_ptr<CSession> new_session, boost::system::error_code ec);

	boost::asio::io_context& _ioc;
	tcp::acceptor _acceptor;
	std::map<std::string, std::shared_ptr<CSession>> _sessions;
	std::mutex _mtx;
};

