#include "AsioIOContextPool.h"

AsioIOContextPool::AsioIOContextPool(std::size_t size) : _IOContexts(size), _works(size) {
	for (std::size_t i = 0; i < size; i++) {
		_works.emplace_back(std::make_unique<Work>(_IOContexts[i].get_executor()));
	}

	for (std::size_t i = 0; i < size; i++) {
		_threads.emplace_back(std::thread([this, i]() {
			_IOContexts[i].run();
			}));
	}
}

AsioIOContextPool::~AsioIOContextPool() {
	Close();
	std::cout << "AsioIOContextPool destruct" << std::endl;
}

boost::asio::io_context& AsioIOContextPool::GetIOC() {
	auto& ioContext = _IOContexts[_nextIOContext++];
	if (_nextIOContext == _IOContexts.size()) {
		_nextIOContext = 0;
	}
	return ioContext;
}

void AsioIOContextPool::Close() {
	for (auto& work : _works) {
		work.reset();
	}

	for (auto& t : _threads) {
		t.join();
	}
}
