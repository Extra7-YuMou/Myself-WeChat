#include "CServer.h"

CServer::CServer(boost::asio::io_context& ioc, unsigned short port) : _ioc(ioc), _sock(ioc),
	_acceptor(ioc, tcp::endpoint(tcp::v4(), port)) {
}

void CServer::StartAccept() {
	auto self = shared_from_this();
	auto pool = AsioIOContextPool::GetInstance();
	auto new_conn = std::make_shared<HttpConnection>(pool->GetIOContext());
	_acceptor.async_accept(new_conn->GetSocket(), [self, new_conn](boost::system::error_code ec) {
		try {
			if (ec) {
				// 出错放弃连接，重新等待新的连接
				std::cout << "Error occured! Error code is " << ec.value()
					<< ", Message is " << ec.message() << std::endl;
				self->StartAccept();
				return;
			}

			new_conn->Start();
			self->StartAccept();
		}
		catch (std::exception& e) {
			std::cout << "Error occured! error is " << e.what() << std::endl;
			return;
		}
		});
}