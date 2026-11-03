#include "CServer.h"
#include "AsioIOContextPool.h"
#include "ConfigMgr.h"
#include "RedisMgr.h"

CServer::CServer(boost::asio::io_context& ioc, unsigned short port) : _ioc(ioc), _timer(ioc, std::chrono::seconds(60)),
	_acceptor(ioc, tcp::endpoint(boost::asio::ip::address_v4::any(), port)){
	startAccept();
	_timer.async_wait([this](boost::system::error_code ec) {
		on_timer(ec);
		});
}

void CServer::on_timer(boost::system::error_code ec){
	if (!ec) {
		std::cout << "Timer Error occured! Error code: " << ec.value()
			<< ", Message: " << ec.message() << std::endl;
	}

	std::vector<std::shared_ptr<CSession>> _expired_sessions;
	int _session_count = 0;
	{
		std::lock_guard<std::mutex> lock(_mtx);
		for (auto iter = _sessions.begin(); iter != _sessions.end(); ++iter) {
			std::time_t now = std::time(nullptr);
			bool _b_expired = iter->second->IsHeartBeatExpried(now);
			if (_b_expired) {
				iter->second->close();
				_expired_sessions.push_back(iter->second);
			}
			else {
				_session_count++;
			}
		}
	}

	auto self_server = ConfigMgr::GetInstance()->getSelfServerName();
	std::string valid_count = std::to_string(_session_count);
	RedisMgr::GetInstance()->HSet(LOGIN_COUNT, self_server, valid_count);

	for (auto& session : _expired_sessions) {
		session->DealExceptionSession();
	}

	_timer.expires_after(std::chrono::seconds(60));
	_timer.async_wait([this](boost::system::error_code ec) {
		on_timer(ec);
		});
}

void CServer::startAccept(){
	auto& ioc = AsioIOContextPool::GetInstance()->GetIOC();
	auto new_session = std::make_shared<CSession>(ioc, this);
	_acceptor.async_accept(new_session->getSocket(), std::bind(&CServer::handlerAccept, this, new_session, std::placeholders::_1));
}

void CServer::handlerAccept(std::shared_ptr<CSession> new_session, boost::system::error_code ec){
	try {
		if (ec) {
			startAccept();
			return;
		}
		new_session->start();
		{
			std::lock_guard<std::mutex> lock(_mtx);
			_sessions.insert(make_pair(new_session->getUuid(), new_session));
		}
	}
	catch (std::exception& e) {
		std::cout << "Error occured! Error is " << e.what() << std::endl;
	}
	startAccept();
}
