#include "AsioIOContextPool.h"
#include "CServer.h"
#include "ConfigMgr.h"

int main() {
	try {
		auto config = ConfigMgr::GetInstance();
		std::string port_str = (*config)["SelfServer"]["Port"];
		unsigned short port = std::atoi(port_str.c_str());
		auto pool = AsioIOContextPool::GetInstance();
		boost::asio::io_context ioc{ 1 };
		boost::asio::signal_set s_set(ioc, SIGINT, SIGTERM);
		s_set.async_wait([&ioc, pool](auto, auto) {
			ioc.stop();
			pool->Close();
			});
		CServer s(ioc, port);
		s.Start();
		std::cout << "Server has listen the port: " << port << std::endl;
		ioc.run();
	}
	catch (std::exception& e) {
		std::cout << "Error occured! Error: " << e.what() << std::endl;
	}
}
