#include "LogicSystem.h"
#include "HttpConnection.h"
#include "VerifyGrpcClient.h"
#include "RedisMgr.h"
#include "MysqlMgr.h"
#include "StatusGrpcClient.h"

LogicSystem::LogicSystem() {
    RegGet("/get_test", [](std::shared_ptr<HttpConnection> connection) {
        beast::ostream(connection->_response.body()) << "receive get_test req " << std::endl;
        int i = 0;
        for (auto& elem : connection->_get_params) {
            i++;
            beast::ostream(connection->_response.body()) << "param" << i << " key is " << elem.first;
            beast::ostream(connection->_response.body()) << ", " << " value is " << elem.second << std::endl;
        }
        });

    RegPost("/get_verify", [](std::shared_ptr<HttpConnection> connection) {
        auto body_str = beast::buffers_to_string(connection->_request.body().data());
        std::cout << "Receive data is " << body_str << std::endl;
        connection->_response.set(http::field::content_type, "text/json");

        Json::Value src_root, root;
        Json::Reader reader;
        bool parse_result = reader.parse(body_str, src_root);
        if (!parse_result) {
            std::cout << "Get verify json 解析失败" << std::endl;
            root["error"] = ErrorCodes::ERR_JSON;
            std::string returnStr = root.toStyledString();
            beast::ostream(connection->_response.body()) << returnStr;
            return;
        }

        auto email = src_root["email"].asString();
        std::cout << "Email is " << email << std::endl;
        GetVerifyRsp rsp = VerifyGrpcClient::GetInstance()->GetVerifyCode(email);
        root["error"] = ErrorCodes::SUCCESS;
        root["email"] = email;
        std::string returnStr = root.toStyledString();
        beast::ostream(connection->_response.body()) << returnStr;
        });

    RegPost("/user_register", [](std::shared_ptr<HttpConnection> connection) {
        auto body_str = beast::buffers_to_string(connection->_request.body().data());
        std::cout << "Receive data is " << body_str << std::endl;
        connection->_response.set(http::field::content_type, "text/json");

        Json::Value src_root, root;
        Json::Reader reader;
        bool _b_parse = reader.parse(body_str, src_root);
        if (!_b_parse) {
            std::cout << "User register json 解析失败" << std::endl;
            root["error"] = ErrorCodes::ERR_JSON;
            std::string returnStr = root.toStyledString();
            beast::ostream(connection->_response.body()) << returnStr;
            return;
        }

        std::string user = src_root["user"].asString();
        std::string email = src_root["email"].asString();
        std::string pwd = src_root["pwd"].asString();
        std::string verify = src_root["verifycode"].asString();

        // 先验证验证码是否正确
        std::string verifyCode;
        bool _b_get_verify = RedisMgr::GetInstance()->Get("code_" + email, verifyCode);
        if (!_b_get_verify) {
            std::cout << "VerifyCode has expired" << std::endl; 
            root["error"] = ErrorCodes::VerifyExpired;
            std::string returnStr = root.toStyledString();
            beast::ostream(connection->_response.body()) << returnStr;
            return;
        }

        if (verifyCode != src_root["verifycode"].asString()) {
            std::cout << "VerifyCode error " << std::endl;
            root["error"] = ErrorCodes::VerifyCodeErr;
            std::string returnStr = root.toStyledString();
            beast::ostream(connection->_response.body()) << returnStr;
            return;
        }

        bool _b_user_exists = RedisMgr::GetInstance()->ExistsKey(src_root["user"].asString());
        if (_b_user_exists) {
            std::cout << "User has exists" << std::endl;
            root["error"] = ErrorCodes::UserExists;
            std::string returnStr = root.toStyledString();
            beast::ostream(connection->_response.body()) << returnStr;
            return;
        }

        int result = MysqlMgr::GetInstance()->RegUser(src_root["user"].asString(), src_root["email"].asString(), src_root["passwd"].asString());
        if (result == 0 || result == -1) {
            std::cout << "User register failure!" << std::endl;
            root["error"] = ErrorCodes::UserExists;
            std::string returnStr = root.toStyledString();
            beast::ostream(connection->_response.body()) << returnStr;
            return;
        }

        root["error"] = ErrorCodes::SUCCESS;
        root["email"] = email;
        root["uuid"] = result;
        std::string returnStr = root.toStyledString();
        beast::ostream(connection->_response.body()) << returnStr;
        });

    RegPost("/reset_pwd", [](std::shared_ptr<HttpConnection> connection) {
        auto body_str = beast::buffers_to_string(connection->_request.body().data());
        std::cout << "Receive data is " << body_str << std::endl;
        connection->_response.set(http::field::content_type, "text/json");

        Json::Value src_root, root;
        Json::Reader reader;
        bool parse_result = reader.parse(body_str, src_root);
        if (!parse_result) {
            std::cout << "Reset pwd json 数据解析失败" << std::endl;
            root["error"] = ErrorCodes::ERR_JSON;
            std::string returnStr = root.toStyledString();
            beast::ostream(connection->_response.body()) << returnStr;
            return;
        }

        std::string email = src_root["email"].asString();
        std::string user = src_root["user"].asString();
        std::string newPwd = src_root["newPwd"].asString();
        std::string verifycode = src_root["verifycode"].asString();
        std::string verify;
        bool _b_get_verify = RedisMgr::GetInstance()->Get(code_prefix + email, verify);
        if (!_b_get_verify) {
            std::cout << "verifyCode has expired" << std::endl;
            root["error"] = ErrorCodes::VerifyExpired;
            std::string returnStr = root.toStyledString();
            beast::ostream(connection->_response.body()) << returnStr;
            return;
        }

        if (verify != verifycode) {
            std::cout << "verifyCode error, verifyCode: " << verify << std::endl;
            root["error"] = ErrorCodes::VerifyCodeErr;
            std::string returnStr = root.toStyledString();
            beast::ostream(connection->_response.body()) << returnStr;
            return;
        }

        bool _b_exists_email = MysqlMgr::GetInstance()->CheckEmail(user, email);
        if (!_b_exists_email) {
            std::cout << "Email not match" << std::endl;
            root["error"] = ErrorCodes::EmailNotMatch;
            std::string returnStr = root.toStyledString();
            beast::ostream(connection->_response.body()) << returnStr;
            return;
        }

        bool _b_reset_pwd = MysqlMgr::GetInstance()->ResetPwd(user, newPwd);
        if (!_b_reset_pwd) {
            std::cout << "Failed to reset pwd" << std::endl;
            root["error"] = ErrorCodes::ResetPwdFailed;
            std::string returnStr = root.toStyledString();
            beast::ostream(connection->_response.body()) << returnStr;
            return;
        }

        std::cout << "Reset " << user << "'s pwd success" << std::endl;
        root["error"] = ErrorCodes::SUCCESS;
        root["user"] = user;
        root["email"] = email;
        std::string returnStr = root.toStyledString();
        beast::ostream(connection->_response.body()) << returnStr;
        });

    RegPost("/user_login", [](std::shared_ptr<HttpConnection> connection) {
        auto body_str = beast::buffers_to_string(connection->_request.body().data());
        std::cout << "Receive data is " << body_str << std::endl;
        connection->_response.set(http::field::content_type, "text/json");

        Json::Value src_root, root;
        Json::Reader reader;
        bool parse_result = reader.parse(body_str, src_root);
        if (!parse_result) {
            std::cout << "User login Json解析失败" << std::endl;
            root["error"] = ErrorCodes::ERR_JSON;
            std::string returnStr = root.toStyledString();
            beast::ostream(connection->_response.body()) << returnStr;
            return;
        }

        std::string name = src_root["user"].asString();
        std::string pwd = src_root["pwd"].asString();
        UserInfo userInfo;
        bool _b_check_pwd = MysqlMgr::GetInstance()->CheckPwd(name, pwd, userInfo);
        if (!_b_check_pwd) {
            std::cout << "User pwd not match" << std::endl;
            root["error"] = ErrorCodes::PasswdInvalid;
            std::string returnStr = root.toStyledString();
            beast::ostream(connection->_response.body()) << returnStr;
            return;
        }

        // 连接状态服务器获取ChatServer
        GetChatServerRsp reply = StatusGrpcClient::GetInstance()->GetChatServer(userInfo.uid);
        if (reply.error() != ErrorCodes::SUCCESS) {
            std::cout << "Grpc get ChatServer failed, error is " << reply.error() << std::endl;
            root["error"] = ErrorCodes::ERR_RPCFialed;
            std::string returnStr = root.toStyledString();
            beast::ostream(connection->_response.body()) << returnStr;
            return;
        }

        // 获取ResourceServer
        GetResourceServerRsp rsp = StatusGrpcClient::GetInstance()->GetResourceServer();
        if (rsp.error() != ErrorCodes::SUCCESS) {
            std::cout << "Grpc get resourceserver failed, error is " << rsp.error() << std::endl;
            root["error"] = ErrorCodes::ERR_RPCFialed;
            beast::ostream(connection->_response.body()) << root.toStyledString();
            return;
        }

        root["user"] = userInfo.name;
        root["uid"] = userInfo.uid;
        root["host"] = reply.host();
        root["port"] = reply.port();
        root["token"] = reply.token();
        root["res_host"] = rsp.host();
        root["res_port"] = rsp.port();
        std::string returnStr = root.toStyledString();
        beast::ostream(connection->_response.body()) << returnStr;
        return;
        });
}

LogicSystem::~LogicSystem()
{
}

bool LogicSystem::HandlerReq(std::string url, std::shared_ptr<HttpConnection> connection){
	if (_get_handlers.find(url) == _get_handlers.end()) {
		return false;
	}

	_get_handlers[url](connection);
	return true;
}

void LogicSystem::RegGet(std::string url, HttpHandler _handler){
	_get_handlers.insert(make_pair(url, _handler));
}

bool LogicSystem::HandlerPost(std::string url, std::shared_ptr<HttpConnection> connection)
{
    if (_post_handlers.find(url) == _post_handlers.end()) {
        return false;
    }

    _post_handlers[url](connection);
    return true;
}

void LogicSystem::RegPost(std::string url, HttpHandler _handler){
    _post_handlers.insert(make_pair(url, _handler));
}

