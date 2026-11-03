#include "CServer.h"
#include "AsioIOContextPool.h"

CServer::CServer(boost::asio::io_context& ioc, unsigned short port) : _ioc(ioc),
	_acceptor(ioc, tcp::endpoint(boost::asio::ip::address_v4::any(), port))
{}

void CServer::Start(){
	auto& ioc = AsioIOContextPool::GetInstance()->GetIOC();
	auto session = std::make_shared<CSession>(ioc, this);
	_acceptor.async_accept(session->GetSocket(), [this, session](boost::system::error_code ec) {
		HandleAccept(session, ec);
		});
}

void CServer::ClearSession(const std::string& uuid){
	{
		std::lock_guard<std::mutex> lcok(_mtx);
		_sessions.erase(uuid);
	}
}

void CServer::HandleAccept(std::shared_ptr<CSession> new_session, boost::system::error_code ec){
	if (!ec) {
		{
			std::lock_guard<std::mutex> lock(_mtx);
			_sessions.insert(make_pair(new_session->GetUuid(), new_session));
		}
		new_session->Start();
	}
	else {
		std::cout << "Error occured! Error code: " << ec.value() << ", Msg: " << ec.message() << std::endl;
	}
	Start();
}
