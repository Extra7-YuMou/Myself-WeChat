#pragma once
#include "Global.h"
#include "Singleton.h"

class AsioIOContextPool : public Singleton<AsioIOContextPool>{
	friend class Singleton<AsioIOContextPool>;
public:
	using _IOContext = boost::asio::io_context;
	using Work = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;
	using WorkPtr = std::shared_ptr<Work>;

	~AsioIOContextPool();
	boost::asio::io_context& GetIOContext();
	void stop();
private:
	AsioIOContextPool(std::size_t size = std::thread::hardware_concurrency());

	std::vector<_IOContext> _IOContexts;
	std::vector<std::thread> _threads;
	std::vector<WorkPtr> _works;
	std::size_t _nextIOContexts;
};

