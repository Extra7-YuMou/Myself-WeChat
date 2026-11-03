#include "AsioIOContextPool.h"

AsioIOContextPool::AsioIOContextPool(std::size_t size) : _IOContexts(size), _nextIOContexts(0) {
    for (std::size_t i = 0; i < size; i++) {
        _works[i] = std::make_unique<Work>(_IOContexts[i].get_executor());
    }

    for (std::size_t i = 0; i < size; i++) {
        _threads.emplace_back([this, i]() {
            _IOContexts[i].run();
        });
    }
}

boost::asio::io_context& AsioIOContextPool::GetIOContext(){
    auto& ioc = _IOContexts[_nextIOContexts++];
    if (_nextIOContexts == _IOContexts.size()) {
        _nextIOContexts = 0;
    }
    return ioc;
}

void AsioIOContextPool::Stop(){
    for (auto& work : _works) {
        work.reset();
    }

    for (auto& t : _threads) {
        if (t.joinable()) {
            t.join();
        }
    }
}
