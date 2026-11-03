#pragma once
#include <memory>
#include <mutex>

template<typename T>
class Singleton {
public:
	Singleton() = default;
	static std::shared_ptr<T> GetInstance() {
		static std::once_flag s_flag;
		std::call_once(s_flag, [&]() {
			_instance = std::shared_ptr<T>(new T);
			});
		return _instance;
	}
	~Singleton() = default;
protected:
	Singleton(const Singleton<T>&) = delete;
	Singleton& operator=(const Singleton<T>&) = delete;
	static std::shared_ptr<T> _instance;
};

template<typename T>
std::shared_ptr<T> Singleton<T>::_instance = nullptr;