#pragma once
#include <boost/asio.hpp>
#include <thread>
#include <vector>
#include <memory>
#include "Singleton.h"

class AsioIOContextPool : Singleton<AsioIOContextPool>{
	friend class Singleton<AsioIOContextPool>;
public:
	using _IOContext = boost::asio::io_context;
	using Work = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;
	using WorkPtr = std::unique_ptr<Work>;
	~AsioIOContextPool() = default;
	boost::asio::io_context& GetIOContext();
	void Stop();
private:
	AsioIOContextPool(std::size_t size = std::thread::hardware_concurrency());
	AsioIOContextPool(const AsioIOContextPool&) = delete;
	AsioIOContextPool& operator=(const AsioIOContextPool&) = delete;

	std::vector<_IOContext> _IOContexts;
	std::vector<WorkPtr> _works;
	std::vector<std::thread> _threads;
	std::size_t _nextIOContexts;
};

