#include "ResourceConPool.h"

ResourceConPool::ResourceConPool(const std::string& host, const std::string& port, const std::size_t& size): _b_stop(false),
_host(host), _port(port), _size(size){
	for (int i = 0; i < size; i++) {
		std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(host + ":" + port, grpc::InsecureChannelCredentials());
		_stubs.push(std::make_unique<ResourceService::Stub>(channel));
	}
}

ResourceConPool::~ResourceConPool(){
	std::unique_lock<std::mutex> lock(_mtx);
	while (!_stubs.empty()) {
		_stubs.pop();
	}
}

void ResourceConPool::Close() {
	_b_stop = true;
	_cond.notify_all();
}

std::unique_ptr<ResourceService::Stub> ResourceConPool::GetStub(){
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

void ResourceConPool::ReturnStub(std::unique_ptr<ResourceService::Stub> stub){
	std::unique_lock<std::mutex> lock(_mtx);
	_stubs.push(std::move(stub));
	_cond.notify_one();
}




