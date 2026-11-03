#include "CSession.h"
#include "CServer.h"
#include "LogicSystem.h"

CSession::CSession(boost::asio::io_context& ioc, CServer* server): _sock(ioc), _server(server), _b_recv_head(false){
	_uuid = boost::uuids::to_string(boost::uuids::random_generator()());
	_recv_head_node = std::make_shared<MsgNode>(TCP_TOTAL_LEN);
}

void CSession::Start() {
	/*auto self = shared_from_this();
	_sock.async_read_some(boost::asio::buffer(_buffer, MAX_BUFFER_LEN),
		std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, self));*/
	AsyncReadHead();
}

void CSession::Send(const std::string& msg, short id){
	if (_send_que.size() > MAX_SEND_QUE_SIZE) {
		std::cout << "Send que is max, max size: " << MAX_SEND_QUE_SIZE << std::endl;
		return;
	}

	{
		std::lock_guard<std::mutex> lock(_mtx);
		_send_que.push(std::make_shared<SendNode>(msg.c_str(), id, msg.length()));
	}
	if (_send_que.size() > 1) {
		return;
	}

	auto self = shared_from_this();
	auto& msgNode = _send_que.front();
	boost::asio::async_write(_sock, boost::asio::buffer(msgNode->_msg, msgNode->_total_len), 
		std::bind(&CSession::HandleWrite, this, std::placeholders::_1, std::placeholders::_2, self));
}

void CSession::HandleRead(boost::system::error_code ec, std::size_t bytes_transferred, std::shared_ptr<CSession> self){
	if (!ec) {
		int copy_len = 0;
		while (bytes_transferred > 0) {
			if (!_b_recv_head) {
				std::cout << "recv_head_node cur_len: " << _recv_head_node->_cur_len << std::endl;
				if (bytes_transferred + _recv_head_node->_cur_len < TCP_TOTAL_LEN) {
					memcpy(_recv_head_node->_msg + _recv_head_node->_cur_len, _buffer + copy_len, bytes_transferred);
					_recv_head_node->_cur_len += bytes_transferred;
					memset(_buffer, '\0', MAX_BUFFER_LEN);
					_sock.async_read_some(boost::asio::buffer(_buffer, MAX_BUFFER_LEN),
						std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, self));
					return;
				}

				int head_remain = TCP_TOTAL_LEN - _recv_head_node->_cur_len;
				memcpy(_recv_head_node->_msg + _recv_head_node->_cur_len, _buffer + copy_len, head_remain);
				_recv_head_node->_cur_len += head_remain;
				copy_len += head_remain;
				bytes_transferred -= head_remain;

				short id = 0;
				memcpy(&id, _recv_head_node->_msg, TCP_ID_LEN);
				id = boost::asio::detail::socket_ops::network_to_host_short(id);
				std::cout << "id: " << id << std::endl;
				if (id > MAX_VALID_ID) {
					std::cout << "id无效, id: " << id << std::endl;
					_server->ClearSession(_uuid);
					return;
				}
				int len = 0;
				memcpy(&len, _recv_head_node->_msg + TCP_ID_LEN, TCP_BODY_LEN);
				len = boost::asio::detail::socket_ops::network_to_host_long(len);
				if (len > MAX_VALID_BODY) {
					std::cout << "len无效" << std::endl;
					_server->ClearSession(_uuid);
					return;
				}
				std::cout << "len: " << len << std::endl;
				_recv_msg_node = std::make_shared<RecvNode>(len, id);

				if (_recv_msg_node->_cur_len + bytes_transferred < len) {
					memcpy(_recv_msg_node->_msg + _recv_msg_node->_cur_len, _buffer + copy_len, bytes_transferred);
					_recv_msg_node->_cur_len += bytes_transferred;
					memset(_buffer, '\0', MAX_BUFFER_LEN);
					_sock.async_read_some(boost::asio::buffer(_buffer, MAX_BUFFER_LEN),
						std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, self));
					_b_recv_head = true;
					return;
				}
				int msg_remain = _recv_msg_node->_total_len - _recv_msg_node->_cur_len;
				memcpy(_recv_msg_node->_msg + _recv_msg_node->_cur_len, _buffer + copy_len, msg_remain);
				_recv_msg_node->_cur_len += msg_remain;
				copy_len += msg_remain;
				bytes_transferred -= msg_remain;
				_recv_msg_node->_msg[_recv_msg_node->_total_len] = '\0';
				std::cout << "Server has receive message, message is " << _recv_msg_node->_msg << std::endl;

				std::hash<std::string> hash_str;
				auto hash_value = hash_str(_uuid);
				int index = hash_value % FILE_WORKER_COUNT;
				std::cout << "index: " << index << std::endl;
				// 调用逻辑系统发送数据
				LogicSystem::GetInstance()->PostToQue(std::make_shared<LogicNode>(self, _recv_msg_node), index);

				_b_recv_head = false;
				_recv_head_node->Clear();
				if (bytes_transferred <= 0) {
					memcpy(_recv_head_node->_msg + _recv_head_node->_cur_len, _buffer + copy_len, bytes_transferred);
					_recv_head_node->_cur_len += bytes_transferred;
					memset(_buffer, '\0', MAX_BUFFER_LEN);
					_sock.async_read_some(boost::asio::buffer(_buffer, MAX_BUFFER_LEN),
						std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, self));
					return;
				}
				continue;
			}

			if (_recv_msg_node->_cur_len + bytes_transferred < _recv_msg_node->_total_len) {
				memcpy(_recv_msg_node->_msg + _recv_msg_node->_cur_len, _buffer + copy_len, bytes_transferred);
				_recv_msg_node->_cur_len += bytes_transferred;
				memset(_buffer, '\0', MAX_BUFFER_LEN);
				_sock.async_read_some(boost::asio::buffer(_buffer, MAX_BUFFER_LEN),
					std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, self));
				return;
			}
			int msg_remain = _recv_msg_node->_total_len - _recv_msg_node->_cur_len;
			memcpy(_recv_msg_node->_msg + _recv_msg_node->_cur_len, _buffer + copy_len, msg_remain);
			_recv_msg_node->_cur_len += msg_remain;
			copy_len += msg_remain;
			bytes_transferred -= msg_remain;
			_recv_msg_node->_msg[_recv_msg_node->_total_len] = '\0';
			std::cout << "Server has receive message, message is " << _recv_msg_node->_msg << std::endl;

			std::hash<std::string> hash_str;
			auto hash_value = hash_str(_uuid);
			int index = hash_value % FILE_WORKER_COUNT;
			LogicSystem::GetInstance()->PostToQue(std::make_shared<LogicNode>(self, _recv_msg_node), index);

			_b_recv_head = false;
			_recv_head_node->Clear();
			if (bytes_transferred <= 0) {
				memcpy(_recv_head_node->_msg + _recv_head_node->_cur_len, _buffer + copy_len, bytes_transferred);
				_recv_head_node->_cur_len += bytes_transferred;
				memset(_buffer, '\0', MAX_BUFFER_LEN);
				_sock.async_read_some(boost::asio::buffer(_buffer, MAX_BUFFER_LEN),
					std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, self));
				return;
			}
			continue;
		}
	}
	else {
		std::cout << "CSession::handleRead 出现错误" << std::endl;
		_server->ClearSession(_uuid);
	}
}

void CSession::AsyncReadHead(){
	auto self = shared_from_this();
	AsyncReadFull(TCP_TOTAL_LEN, [this, self](const boost::system::error_code ec, std::size_t bytes_transferred) {
		if (ec) {
			std::cout << "Read tcp head occured error, error code: " << ec.value()
				<< ", msg: " << ec.message() << std::endl;
			_server->ClearSession(_uuid);
			return;
		}

		if (bytes_transferred < TCP_TOTAL_LEN) {
			std::cout << "Read length not match, read length [ " << bytes_transferred << " ] total length [ "
				<< TCP_TOTAL_LEN << " ]" << std::endl;
			_server->ClearSession(_uuid);
			return;
		}

		_recv_head_node->Clear();
		memcpy(_recv_head_node->_msg, _buffer, TCP_TOTAL_LEN);
		_recv_head_node->_cur_len += bytes_transferred;

		short msg_id = 0;
		memcpy(&msg_id, _recv_head_node->_msg, TCP_ID_LEN);
		msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
		if (msg_id > MAX_VALID_ID) {
			std::cout << "ID invalid, ID: " << msg_id << std::endl;
			_server->ClearSession(_uuid);
			return;
		}

		int msg_len = 0;
		memcpy(&msg_len, _recv_head_node->_msg + TCP_ID_LEN, TCP_BODY_LEN);
		msg_len = boost::asio::detail::socket_ops::network_to_host_long(msg_len);
		if (msg_len > MAX_VALID_BODY) {
			std::cout << "msg len invalid, len: " << msg_len << std::endl;
			_server->ClearSession(_uuid);
			return;
		}
		std::cout << "Receive ID: " << msg_id << ", msg len: " << msg_len << std::endl;

		_recv_msg_node = std::make_shared<RecvNode>(msg_id, msg_len);
		AsyncReadBody(msg_len);
		});
}

void CSession::AsyncReadBody(std::size_t total_len){
	auto self = shared_from_this();
	AsyncReadFull(total_len, [this, self, total_len](const boost::system::error_code ec, std::size_t bytes_transferred) {
		if (ec) {
			std::cout << "Read body occured error, Error code: " << ec.value() << ", msg: "
				<< ec.message() << std::endl;
			_server->ClearSession(_uuid);
			return;
		}

		if (bytes_transferred < total_len) {
			std::cout << "Read body len not match, read len [ " << bytes_transferred << " ] total len [ "
				<< total_len << " ] " << std::endl;
			_server->ClearSession(_uuid);
			return;
		}

		memcpy(_recv_msg_node->_msg, _buffer, bytes_transferred);
		_recv_msg_node->_cur_len += bytes_transferred;
		_recv_msg_node->_msg[_recv_msg_node->_total_len] = '\0';
		std::cout << "Server has receive msg, msg is " << self->_recv_msg_node->_msg << std::endl;

		std::hash<std::string> hash_str;
		auto value = hash_str(_uuid);
		int index = value % LOGIC_WORKER_COUNT;
		LogicSystem::GetInstance()->PostToQue(std::make_shared<LogicNode>(self, _recv_msg_node), index);

		AsyncReadHead();
		});
}

void CSession::AsyncReadFull(std::size_t total_len, std::function<void(const boost::system::error_code, std::size_t bytes_transferred)> handler){
	memset(_buffer, '\0', MAX_BUFFER_LEN);
	AsyncReadLen(0, total_len, handler);
}

void CSession::AsyncReadLen(std::size_t read_len, std::size_t total_len, std::function<void(const boost::system::error_code, std::size_t bytes_transferred)> handler){
	auto self = shared_from_this();
	_sock.async_read_some(boost::asio::buffer(_buffer + read_len, total_len - read_len),
		[self, read_len, total_len, handler](const boost::system::error_code ec, std::size_t bytes_transferred) {
			if (ec) {
				handler(ec, bytes_transferred);
				return;
			}

			if (read_len + bytes_transferred >= total_len) {
				handler(ec, read_len + bytes_transferred);
				return;
			}

			self->AsyncReadLen(read_len + bytes_transferred, total_len, handler);
		});
}

void CSession::HandleWrite(boost::system::error_code ec, std::size_t bytes_transferred, std::shared_ptr<CSession> self){
	if (!ec) {
		std::cout << "Server has reply, reply is: " << (_send_que.front()->_msg + TCP_TOTAL_LEN) << std::endl;
		{
			std::lock_guard<std::mutex> lock(_mtx);
			_send_que.pop();
		}
		if (_send_que.size() > 0) {
			auto& msgNode = _send_que.front();
			boost::asio::async_write(_sock, boost::asio::buffer(msgNode->_msg, msgNode->_total_len), 
				std::bind(&CSession::HandleWrite, this, std::placeholders::_1, std::placeholders::_2, self));
		}
	}
	else {
		std::cout << "Error occured! Error code: " << ec.value() << ", Msg: " << ec.message() << std::endl;
		_server->ClearSession(_uuid);
	}
}
