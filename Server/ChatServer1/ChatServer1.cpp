#include "CServer.h"
#include "AsioIOContextPool.h"
#include "ConfigMgr.h"
#include "RedisMgr.h"
#include "ChatServiceImpl.h"
#include "ResourceServiceImpl.h"

int main()
{
    try {
        auto config = ConfigMgr::GetInstance();

        auto redis = RedisMgr::GetInstance();
        redis->HSet(LOGIN_COUNT, (*config)["SelfServer"]["Name"], "0");
        std::string grpc_server_address((*config)["SelfServer"]["Host"] + ":" + (*config)["SelfServer"]["RPCPort"]);
        std::cout << "Grpc server has listen the port " << grpc_server_address << std::endl;

        ChatServiceImpl service;
        grpc::ServerBuilder builder;
        builder.AddListeningPort(grpc_server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);

        std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
        std::cout << "Grpc Server has listen the port " << grpc_server_address << std::endl;

        std::thread grpc_server_thread([&server]() {
            server->Wait();
            });

        std::string grpc_res_server_address((*config)["ResourceServer"]["Host"] + ":" + (*config)["ResourceServer"]["RPCPort"]);
        ResourceServiceImpl res_service;
        grpc::ServerBuilder res_builder;
        res_builder.AddListeningPort(grpc_res_server_address, grpc::InsecureServerCredentials());
        res_builder.RegisterService(&res_service);

        std::unique_ptr<grpc::Server> res_server(res_builder.BuildAndStart());
        std::cout << "Resource Grpc Server has listen the port " << grpc_res_server_address << std::endl;

        std::thread grpc_res_server_thread([&res_server]() {
            res_server->Wait();
            });

        short port = std::atoi((*config)["SelfServer"]["Port"].c_str());
        auto pool = AsioIOContextPool::GetInstance();
        boost::asio::io_context ioc{ 1 };
        boost::asio::signal_set s_set{ ioc, SIGINT, SIGTERM };
        s_set.async_wait([&ioc, pool](auto,auto) {
            ioc.stop();
            pool->Close();
            });
        CServer s(ioc, port);
        std::cout << "ChatServer1 has listen the prot " << port << std::endl;
        ioc.run();

        redis->HDel(LOGIN_COUNT, (*config)["SelfServer"]["Name"]);
        redis->Close();
        grpc_server_thread.join();
        grpc_res_server_thread.join();
    }
    catch (std::exception& e) {
        std::cout << "error occured! error is " << e.what() << std::endl;
    }
}
