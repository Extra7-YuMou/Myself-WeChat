#include "ResourceGrpcClient.h"
#include "ConfigMgr.h"
#include "Global.h"

GetChatImgMsgRsp ResourceGrpcClient::NotifyChatImgMsg(const int64_t& msg_id, const int& uid, const int& recv_uid,
    const std::string& name, const int64_t& thread_id, const std::string& md5, const int64_t& file_size){
    ClientContext context;
    GetChatImgMsgReq req;
    GetChatImgMsgRsp rsp;
    req.set_msg_id(msg_id);
    req.set_uid(uid);
    req.set_recv_uid(recv_uid);
    req.set_name(name);
    req.set_thread_id(thread_id);
    req.set_md5(md5);
    req.set_total_size(file_size);

    auto stub = _pool->GetStub();
    Defer defer([&stub, this]() {
        _pool->ReturnStub(std::move(stub));
        });

    Status status = stub->NotifyChatImgMsg(&context, req, &rsp);
    if (!status.ok()) {
        rsp.set_error(ErrorCodes::ERR_RPCFialed);
        return rsp;
    }
    rsp.set_error(ErrorCodes::SUCCESS);
    return rsp;
}

GetChatVideoMsgRsp ResourceGrpcClient::NotifyChatVideoMsg(const int64_t& msg_id, const int& uid, const int& recv_uid, 
    const std::string& name, const std::string& cover_name, const int64_t& thread_id, const int& duration){
    ClientContext context;
    GetChatVideoMsgReq req;
    GetChatVideoMsgRsp rsp;
    req.set_msg_id(msg_id);
    req.set_uid(uid);
    req.set_recv_uid(recv_uid);
    req.set_name(name);
    req.set_thread_id(thread_id);
    req.set_cover_name(cover_name);
    req.set_duration(duration);

    auto stub = _pool->GetStub();
    Defer defer([&stub, this]() {
        _pool->ReturnStub(std::move(stub));
        });

    Status status = stub->NotifyChatVideoMsg(&context, req, &rsp);
    if (!status.ok()) {
        rsp.set_error(ErrorCodes::ERR_RPCFialed);
        return rsp;
    }
    rsp.set_error(ErrorCodes::SUCCESS);
    return rsp;
}

ResourceGrpcClient::ResourceGrpcClient() {
    auto config = ConfigMgr::GetInstance();
    std::string host = (*config)["SelfServer"]["RPCHost"];
    std::string port = (*config)["SelfServer"]["RPCPort"];
    _pool = std::make_unique<ResourceConPool>(host, port, 10);
}