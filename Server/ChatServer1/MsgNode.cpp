#include "MsgNode.h"

RecvNode::RecvNode(short len, short id) : MsgNode(len), _msg_id(id){}

short RecvNode::getId(){
	return _msg_id;
}

SendNode::SendNode(const char* msg, int len, short id) : MsgNode(len + TOTAL_HEAD_LENGTH), _msg_id(id){
    short net_id = boost::asio::detail::socket_ops::host_to_network_short(id);
    memcpy(_msg, &net_id, HEAD_ID_LENGTH);              // 前2字节: id
    std::cout << "SendNode len: " << len << std::endl;
    int net_len = boost::asio::detail::socket_ops::host_to_network_long(len);
    memcpy(_msg + HEAD_ID_LENGTH, &net_len, HEAD_LEN_LENGTH); // 接着2字节: 长度
    memcpy(_msg + TOTAL_HEAD_LENGTH, msg, len);         // 拷贝数据
}
