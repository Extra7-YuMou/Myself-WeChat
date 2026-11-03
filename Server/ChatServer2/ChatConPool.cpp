#include "ChatConPool.h"

ChatConPool::ChatConPool(std::size_t size, const std::string& host, const std::string& port): _size(size), _host(host), _port(port), _b_stop(false){
	for (std::size_t i = 0; i < size; i++) {
		std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(host + ":" + port, grpc::InsecureChannelCredentials());
		_stubs.push(std::make_unique<ChatService::Stub>(channel));
	}
}

std::unique_ptr<ChatService::Stub> ChatConPool::getStub() {
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

void ChatConPool::returnStub(std::unique_ptr<ChatService::Stub> stub) {
	std::unique_lock<std::mutex> lock(_mtx);
	_stubs.push(std::move(stub));
	_cond.notify_one();
}

void ChatConPool::close() {
	_b_stop = true;
	_cond.notify_all();
}

ChatConPool::~ChatConPool(){
	std::unique_lock<std::mutex> lock(_mtx);
	while (!_stubs.empty()) {
		_stubs.pop();
	}
}
