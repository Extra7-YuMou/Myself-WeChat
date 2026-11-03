#pragma once
#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include "Singleton.h"

class AsioIOContextPool : public Singleton<AsioIOContextPool> {
	friend class Singleton<AsioIOContextPool>;
public:
	using IOContext = boost::asio::io_context;
	using Work = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;
	using WorkPtr = std::unique_ptr<Work>;
	~AsioIOContextPool();
	boost::asio::io_context& GetIOC();
	void Close();
private:
	AsioIOContextPool(std::size_t size = std::thread::hardware_concurrency());
	AsioIOContextPool(const AsioIOContextPool&) = delete;
	AsioIOContextPool& operator=(const AsioIOContextPool&) = delete;

	std::vector<IOContext> _IOContexts;
	std::vector<WorkPtr> _works;
	std::vector<std::thread> _threads;
	std::size_t _nextIOContext;
};

