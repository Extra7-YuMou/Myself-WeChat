#include "MsgNode.h"
#include "Global.h"

SendNode::SendNode(const char* msg, short msg_id, int total_len): MsgNode(total_len + TCP_TOTAL_LEN), _msg_id(msg_id){
	short id = boost::asio::detail::socket_ops::host_to_network_short(msg_id);
	memcpy(_msg, &id, TCP_ID_LEN);
	int len = boost::asio::detail::socket_ops::host_to_network_long(total_len);
	memcpy(_msg + TCP_ID_LEN, &len, TCP_BODY_LEN);
	memcpy(_msg + TCP_TOTAL_LEN, msg, total_len);
}
