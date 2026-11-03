#include "StatusConPool.h"

StatusConPool::StatusConPool(std::size_t size, const std::string& host, const std::string& port) : 
	_poolSize(size), _host(host), _port(port), _b_stop(false){
	for (std::size_t i = 0; i < size; i++) {
		auto channel = grpc::CreateChannel(host + ":" + port,
			grpc::InsecureChannelCredentials());
		_stubs.push(StatusService::NewStub(channel));
	}
}

std::unique_ptr<StatusService::Stub> StatusConPool::GetConnection() {
	std::unique_lock<std::mutex> lock(_mtx);
	_cond.wait(lock, [this]() {
		if (_b_stop) {
			return true;
		}
		return !_stubs.empty();
		});

	if (_b_stop) {
		return nullptr;
	}
	auto stub = std::move(_stubs.front());
	_stubs.pop();
	return stub;
}

void StatusConPool::returnConnection(std::unique_ptr < StatusService::Stub> connection) {
	std::unique_lock<std::mutex> lock(_mtx);
	_stubs.push(std::move(connection));
	_cond.notify_one();
}

StatusConPool::~StatusConPool(){
	std::unique_lock<std::mutex> lock(_mtx);
	while (!_stubs.empty()) {
		_stubs.pop();
	}
}

void StatusConPool::Close() {
	_b_stop = true;
	_cond.notify_all();
}