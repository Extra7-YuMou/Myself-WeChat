#include "StatusConPool.h"
#include "ConfigMgr.h"
#include "ChatConPool.h"

StatusConPool::StatusConPool(std::size_t size, const std::string& host, const std::string& port): _b_stop(false),
	_host(host), _port(port){
	for (std::size_t i = 0; i < size; i++) {
		std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(host + ":" + port, grpc::InsecureChannelCredentials());
		_stubs.push(std::make_unique<StatusService::Stub>(channel));
	}
}

std::unique_ptr<StatusService::Stub> StatusConPool::getStub(){
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

void StatusConPool::returnStub(std::unique_ptr<StatusService::Stub> stub){
	std::unique_lock<std::mutex> lock(_mtx);
	_stubs.push(std::move(stub));
	_cond.notify_one();
}

void StatusConPool::Close() {
	_b_stop = true;
	_cond.notify_all();
}

StatusConPool::~StatusConPool(){
	std::unique_lock<std::mutex> lock(_mtx);
	while (!_stubs.empty()) {
		_stubs.pop();
	}
}

