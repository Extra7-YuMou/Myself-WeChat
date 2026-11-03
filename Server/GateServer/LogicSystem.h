#pragma once
#include "Global.h"
#include "Singleton.h"
#include <queue>

class HttpConnection;
class LogicSystem : public Singleton<LogicSystem>{
	friend class Singleton<LogicSystem>;
	using HttpHandler = std::function<void(std::shared_ptr<HttpConnection>)>;
public:
	~LogicSystem();
	bool HandlerReq(std::string url, std::shared_ptr<HttpConnection> connection);
	void RegGet(std::string url, HttpHandler _handler);
	bool HandlerPost(std::string url, std::shared_ptr<HttpConnection> connection);
	void RegPost(std::string url, HttpHandler _handler);
private:
	LogicSystem();

	std::map<std::string, HttpHandler> _get_handlers;
	std::map<std::string, HttpHandler> _post_handlers;
};

