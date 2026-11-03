#pragma once
#include "Global.h"
#include "CSession.h"
#include "UserMgr.h"

class CServer{
public:
	CServer(boost::asio::io_context& ioc, unsigned short port);
	void clearSession(std::string uuid) {
		if (_sessions.find(uuid) != _sessions.end()) {
			UserMgr::GetInstance()->RmvUidSession(_sessions[uuid]->getUid());
		}

		{
			std::lock_guard<std::mutex> lock(_mtx);
			_sessions.erase(uuid);
		}
	}

	void on_timer(boost::system::error_code ec);
private:
	void startAccept();
	void handlerAccept(std::shared_ptr<CSession> new_session, boost::system::error_code ec);

	tcp::acceptor _acceptor;
	boost::asio::io_context& _ioc;
	std::map<std::string, std::shared_ptr<CSession>> _sessions;
	std::mutex _mtx;
	boost::asio::steady_timer _timer;
};

