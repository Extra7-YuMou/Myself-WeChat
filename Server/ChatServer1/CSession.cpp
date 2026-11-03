#include "CSession.h"
#include "CServer.h"
#include "LogicSystem.h"
#include "RedisMgr.h"

CSession::CSession(boost::asio::io_context& ioc, CServer* server) : _sock(ioc), _server(server), _b_recv_head(false){
	_uuid = boost::uuids::to_string(boost::uuids::random_generator()());
	_recv_head_node = std::make_shared<MsgNode>(TOTAL_HEAD_LENGTH);
}

void CSession::start(){
	_sock.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
		std::bind(&CSession::handleRead, this, std::placeholders::_1, std::placeholders::_2, shared_from_this()));
}

void CSession::Send(const std::string& msg, short id){
	std::unique_lock<std::mutex> lock(_mtx);
	if (_send_que.size() > MAX_SEND_SIZE) {
		std::cout << "session: " << _uuid << " send que fulled, size is " << MAX_SEND_SIZE << std::endl;
		return;
	}

	_send_que.push(std::make_shared<SendNode>(msg.c_str(), msg.length(), id));
	if (_send_que.size() > 1) {
		return;
	}

	auto& msgNode = _send_que.front();
	boost::asio::async_write(_sock, boost::asio::buffer(msgNode->_msg, msgNode->_total_len),
		std::bind(&CSession::handleWrite, this, std::placeholders::_1, std::placeholders::_2, shared_from_this()));
}

void CSession::close() {
	std::lock_guard<std::mutex> lock(_mtx);
	_sock.close();
	RedisMgr::GetInstance()->HDel(USERLOGINIP, std::to_string(_uid));
}

void CSession::notifyOffline(int uid){
	Json::Value rtvalue;
	rtvalue["error"] = ErrorCodes::SUCCESS;
	rtvalue["uid"] = uid;
	Send(rtvalue.toStyledString(), MSG_NOTIFY_OFF_LINE);
}

CServer* CSession::getServer(){
	return _server;
}

void CSession::DealExceptionSession(){
	auto identifier = RedisMgr::GetInstance()->acquireLock(std::to_string(_uid), LOCK_TIME_OUT, ACQUIRE_TIME_OUT);
	Defer defer([this, &identifier]() {
		_server->clearSession(_uuid);
		RedisMgr::GetInstance()->releaseLock(std::to_string(_uid), identifier);
		});

	if (identifier.empty()) {
		return;
	}

	std::string sessionIdKey = USER_SESSION_PREFIX + std::to_string(_uid);
	std::string sessionIdValue = "";
	bool _b_get = RedisMgr::GetInstance()->Get(sessionIdKey, sessionIdValue);
	if (!_b_get) {
		return;
	}

	if (sessionIdValue != _uuid) {
		// 其它地方登录
		return;
	}

	RedisMgr::GetInstance()->Del(sessionIdKey);
	RedisMgr::GetInstance()->HDel(USERLOGINIP, sessionIdKey);
}

bool CSession::IsHeartBeatExpried(std::time_t& now){
	std::time_t diff = std::difftime(now, _last_heartbeat);
	if (diff > 60) {
		return true;
	}
	return false;
}

void CSession::UpdateHeartBeat(){
	std::time_t now = std::time(nullptr);
	_last_heartbeat = now;
}

void CSession::handleRead(boost::system::error_code ec, std::size_t bytes_transferred, std::shared_ptr<CSession> _self){
	try {
		if (!ec) {
			short copy_len = 0;
			while (bytes_transferred > 0) {
				if (!_b_recv_head) {
					if (bytes_transferred + _recv_head_node->_cur_len < TOTAL_HEAD_LENGTH) {
						memcpy(_recv_head_node->_msg + _recv_head_node->_cur_len, _data + copy_len, bytes_transferred);
						_recv_head_node->_cur_len += bytes_transferred;
						memset(_data, '\0', MAX_LENGTH);
						_sock.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
							std::bind(&CSession::handleRead, this, std::placeholders::_1, std::placeholders::_2, _self));
						return;
					}

					short head_remain = TOTAL_HEAD_LENGTH - _recv_head_node->_cur_len;
					if (bytes_transferred < head_remain) {
						memcpy(_recv_head_node->_msg + _recv_head_node->_cur_len, _data + copy_len, bytes_transferred);
						_recv_head_node->_cur_len += bytes_transferred;
						memset(_data, '\0', MAX_LENGTH);
						_sock.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
							std::bind(&CSession::handleRead, this, std::placeholders::_1, std::placeholders::_2, _self));
						return;
					}
					memcpy(_recv_head_node->_msg + _recv_head_node->_cur_len, _data + copy_len, head_remain);
					_recv_head_node->_cur_len += head_remain;
					copy_len += head_remain;
					bytes_transferred -= head_remain;

					short id = 0;
					memcpy(&id, _recv_head_node->_msg, HEAD_ID_LENGTH);
					id = boost::asio::detail::socket_ops::network_to_host_short(id);
					int len = 0;
					memcpy(&len, _recv_head_node->_msg + HEAD_ID_LENGTH, HEAD_LEN_LENGTH);
					len = boost::asio::detail::socket_ops::network_to_host_long(len);
					std::cout << "data id is " << id <<", data len is " << len << std::endl;
					if (len > MAX_LENGTH) {
						std::cout << "消息长度超出限制，将断开连接" << std::endl;
						_server->clearSession(_uuid);
						return;
					}
					_recv_msg_node = std::make_shared<RecvNode>(len, id);

					if (bytes_transferred < len) {
						memcpy(_recv_msg_node->_msg + _recv_msg_node->_cur_len, _data + copy_len, bytes_transferred);
						_recv_msg_node->_cur_len += bytes_transferred;
						memset(_data, '\0', MAX_LENGTH);
						_sock.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
							std::bind(&CSession::handleRead, this, std::placeholders::_1, std::placeholders::_2, _self));
						_b_recv_head = true;
						return;
					}

					short msg_remain = _recv_msg_node->_total_len - _recv_msg_node->_cur_len;
					memcpy(_recv_msg_node->_msg + _recv_msg_node->_cur_len, _data + copy_len, msg_remain);
					_recv_msg_node->_cur_len += msg_remain;
					copy_len += msg_remain;
					bytes_transferred -= msg_remain;
					_recv_msg_node->_msg[_recv_msg_node->_cur_len] = '\0';
					std::cout << "Server has receive message, message is " << _recv_msg_node->_msg << std::endl;
					LogicSystem::GetInstance()->PostMsgToQue(std::make_shared<LogicNode>(_self, _recv_msg_node));
					UpdateHeartBeat();

					_b_recv_head = false;
					_recv_head_node->clear();
					if (bytes_transferred <= 0) {
						memcpy(_recv_head_node->_msg + _recv_head_node->_cur_len, _data + copy_len, bytes_transferred);
						_recv_head_node->_cur_len += bytes_transferred;
						memset(_data, '\0', MAX_LENGTH);
						_sock.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
							std::bind(&CSession::handleRead, this, std::placeholders::_1, std::placeholders::_2, _self));
						return;
					}
					continue;
				}

				short msg_remain = _recv_msg_node->_total_len - _recv_msg_node->_cur_len;
				if (bytes_transferred < msg_remain) {
					memcpy(_recv_msg_node->_msg + _recv_msg_node->_cur_len, _data + copy_len, bytes_transferred);
					_recv_msg_node->_cur_len += bytes_transferred;
					memset(_data, '\0', MAX_LENGTH);
					_sock.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
						std::bind(&CSession::handleRead, this, std::placeholders::_1, std::placeholders::_2, _self));
					return;
				}
				memcpy(_recv_msg_node->_msg + _recv_msg_node->_cur_len, _data + copy_len, msg_remain);
				_recv_msg_node->_cur_len += msg_remain;
				copy_len += msg_remain;
				bytes_transferred -= msg_remain;
				_recv_msg_node->_msg[_recv_msg_node->_cur_len] = '\0';
				std::cout << "Server has receive message, message is " << _recv_msg_node->_msg << std::endl;
				LogicSystem::GetInstance()->PostMsgToQue(std::make_shared<LogicNode>(_self, _recv_msg_node));
				UpdateHeartBeat();

				_b_recv_head = false;
				_recv_head_node->clear();
				if (bytes_transferred <= 0) {
					memcpy(_recv_msg_node->_msg + _recv_msg_node->_cur_len, _data + copy_len, bytes_transferred);
					_recv_msg_node->_cur_len += bytes_transferred;
					memset(_data, '\0', MAX_LENGTH);
					_sock.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
						std::bind(&CSession::handleRead, this, std::placeholders::_1, std::placeholders::_2, _self));
					return;
				}
				continue;
			}
		}
		else {
			std::cout << "Error occured! Error code: " << ec.value() << ", Message: " << ec.message() << std::endl;
			close();

			DealExceptionSession();
		}
	}
	catch (std::exception& e) {
		std::cout << "Error occured! Error is " << e.what() << std::endl;
	}
}

void CSession::handleWrite(boost::system::error_code ec, std::size_t bytes_transferred, std::shared_ptr<CSession> _self){
	try {
		if (!ec) {
			UpdateHeartBeat();
			//std::cout << "Server has reply, reply is " << _recv_msg_node->_msg << std::endl;
			std::cout << "Server has reply, reply is " << (_send_que.front()->_msg + TOTAL_HEAD_LENGTH) << std::endl;
			_send_que.pop();
			if (!_send_que.empty()) {
				auto& msgNode = _send_que.front();
				boost::asio::async_write(_sock, boost::asio::buffer(msgNode->_msg, msgNode->_total_len),
					std::bind(&CSession::handleWrite, this, std::placeholders::_1, std::placeholders::_2, _self));
			}
		}
		else {
			std::cout << "Error occured! Error code: " << ec.value() << ", Message: " << ec.message() << std::endl;
			close();

			DealExceptionSession();
		}
	}
	catch (std::exception& e) {
		std::cout << "Error occured! Error is " << e.what() << std::endl;
	}
}


