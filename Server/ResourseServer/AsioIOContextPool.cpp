#include "AsioIOContextPool.h"

AsioIOContextPool::AsioIOContextPool(std::size_t size) :
	_IOCs(size), _works(size), _index(0) {
	for (std::size_t i = 0; i < size; i++) {
		_works[i] = std::make_unique<Work>(_IOCs[i].get_executor());
	}

	for (std::size_t i = 0; i < size; i++) {
		_threads.emplace_back([this, i]() {
			_IOCs[i].run();
			});
	}
}

AsioIOContextPool::~AsioIOContextPool(){
	Close();
}

boost::asio::io_context& AsioIOContextPool::GetIOC(){
	auto& ioc = _IOCs[_index++];
	if (_index == _IOCs.size()) {
		_index = 0;
	}
	return ioc;
}

void AsioIOContextPool::Close() {
	for (auto& work : _works) {
		work.reset();
	}

	for (auto& t : _threads) {
		if (t.joinable()) {
			t.join();
		}
	}
}