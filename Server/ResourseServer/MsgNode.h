#pragma once
#include <boost/asio.hpp>

class MsgNode{
public:
	MsgNode(int total_len) : _cur_len(0), _total_len(total_len) {
		_msg = new char[total_len + 1];
	}

	~MsgNode() {
		delete[] _msg;
	}

	void Clear() {
		memset(_msg, '\0', _total_len);
		_cur_len = 0;
	}

	char* _msg;
	int _total_len;
	int _cur_len;
};

class RecvNode : public MsgNode {
public:
	RecvNode(short msg_id, int total_len): MsgNode(total_len), _msg_id(msg_id){}

	short _msg_id;
};

class SendNode : public MsgNode {
public:
	SendNode(const char* msg, short msg_id, int total_len);

	short _msg_id;
};