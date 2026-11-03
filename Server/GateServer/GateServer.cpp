#include "CServer.h"
#include "ConfigMgr.h"
#include "hiredis.h"
#include "RedisMgr.h"

int main()
{
    try {
        auto configMgr = ConfigMgr::GetInstance();
        configMgr->printAll();
        boost::asio::io_context ioc{ 1 };
        unsigned short port = std::atoi((*configMgr)["GateServer"]["Port"].c_str());
        boost::asio::signal_set flag_set(ioc, SIGINT, SIGTERM);
        flag_set.async_wait([&ioc](auto, auto) {
            ioc.stop();
            });
        std::cout << "HttpServer has listening the port " << port << "." << std::endl;
        auto s = std::make_shared<CServer>(ioc, port);
        s->StartAccept();
        ioc.run();
        
    }
    catch (std::exception& e) {
        std::cout << "Error occured! error is " << e.what() << std::endl;
    }
}
