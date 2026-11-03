#include "CServer.h"
#include "RedisMgr.h"
#include "ConfigMgr.h"
#include "ChatServiceImpl.h"
#include "AsioIOContextPool.h"

int main()
{
    try {
        auto config = ConfigMgr::GetInstance();
        auto selfServer = (*config)["SelfServer"]["Name"];
        RedisMgr::GetInstance()->HSet(LOGIN_COUNT, selfServer, "0");
        std::string grpc_server_address = (*config)["SelfServer"]["Host"] + ":" + (*config)["SelfServer"]["RPCPort"];
        ChatServiceImpl service;
        grpc::ServerBuilder builder;
        builder.AddListeningPort(grpc_server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);

        std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
        std::cout << "Grpc Server has listen the port " << grpc_server_address << std::endl;

        std::thread grpc_server_thread([&server]() {
            server->Wait();
            });

        boost::asio::io_context ioc{ 1 };
        int port = std::atoi((*config)["SelfServer"]["Port"].c_str());
        boost::asio::signal_set s_set(ioc, SIGINT, SIGTERM);
        auto pool = AsioIOContextPool::GetInstance();
        s_set.async_wait([&ioc, pool](auto, auto) {
            ioc.stop();
            pool->Close();
            });
        CServer s(ioc, port);
        std::cout << "ChatServer2 has listen the url " << (*config)["SelfServer"]["Host"] + ":" + (*config)["SelfServer"]["Port"] << std::endl;
        ioc.run();

        RedisMgr::GetInstance()->HDel(LOGIN_COUNT, selfServer);
        RedisMgr::GetInstance()->Close();
        grpc_server_thread.join();
    }
    catch (std::exception& e) {
        std::cerr << "Error occured! Error is " << e.what() << std::endl;
    }
}
