#include "AsioIOContextPool.h"

AsioIOContextPool::~AsioIOContextPool(){}

boost::asio::io_context& AsioIOContextPool::GetIOContext()
{
	auto& ioc = _IOContexts[_nextIOContexts++];
	if (_nextIOContexts == _IOContexts.size()) {
		_nextIOContexts = 0;
	}
	return ioc;
}

AsioIOContextPool::AsioIOContextPool(std::size_t size) : _IOContexts(size), _works(size), _nextIOContexts(0){
	for (std::size_t i = 0; i < size; i++) {
		_works[i] = std::shared_ptr<Work>(new Work(_IOContexts[i].get_executor()));
	}

	for (std::size_t i = 0; i < size; i++) {
		_threads.emplace_back([this, i]() {
			_IOContexts[i].run();
			});
	}
}

void AsioIOContextPool::stop() {
	for (auto& work : _works) {
		work.reset();
	}

	for (auto& t : _threads) {
		if (t.joinable()) {
			t.join();
		}
	}
}
