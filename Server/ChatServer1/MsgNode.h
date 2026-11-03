#pragma once
#include "Global.h"

class MsgNode{
public:
	MsgNode(short len) : _total_len(len), _cur_len(0) {
		_msg = new char[len + 1];
	}

	~MsgNode() {
		delete[] _msg;
	}

	void clear() {
		memset(_msg, '\0', _total_len);
		_cur_len = 0;
	}

	char* _msg;
	int _total_len;
	int _cur_len;
};

class RecvNode : public MsgNode {
public:
	RecvNode(short len, short id);
	short getId();
private:
	short _msg_id;
};

class SendNode : public MsgNode {
public:
	SendNode(const char* msg, int len, short id);
private:
	short _msg_id;
};