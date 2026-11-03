#include "ResourceServiceImpl.h"
#include "UserMgr.h"
#include "Global.h"

Status ResourceServiceImpl::NotifyChatImgMsg(::grpc::ServerContext* context, const ::message::GetChatImgMsgReq* req, ::message::GetChatImgMsgRsp* rsp) {
	auto uid = req->uid();
	auto recv_uid = req->recv_uid();
	auto msg_id = req->msg_id();
	auto name = req->name();
	auto thread_id = req->thread_id();
	auto md5 = req->md5();
	auto file_size = req->total_size();

	auto session = UserMgr::GetInstance()->getUidSession(uid);
	if (session == nullptr) {
		std::cout << "NotifyChatImgMsg User session is nullptr" << std::endl;
		rsp->set_error(ErrorCodes::UidInvalid);
		return Status::OK;
	}

	Json::Value rtvalue;
	rtvalue["error"] = ErrorCodes::SUCCESS;
	rtvalue["md5"] = md5;
	rtvalue["send_uid"] = uid;
	rtvalue["recv_uid"] = recv_uid;
	rtvalue["message_id"] = msg_id;
	rtvalue["name"] = name;
	rtvalue["thread_id"] = thread_id;
	rtvalue["type"] = 2;
	std::cout << "Notify: " << rtvalue.toStyledString() << std::endl;
	session->Send(rtvalue.toStyledString(), MSG_NOTIFY_UPLOAD_IMG_CHAT_RSP);
	rsp->set_error(ErrorCodes::SUCCESS);

	auto recv_session = UserMgr::GetInstance()->getUidSession(recv_uid);
	if (recv_session == nullptr) {
		std::cout << "NotifyChatImgMsg Recv User session is nullptr" << std::endl;
		rsp->set_error(ErrorCodes::ERR_RPCFialed);
		return Status::OK;
	}

	Json::Value recv_value;
	recv_value["error"] = ErrorCodes::SUCCESS;
	recv_value["md5"] = md5;
	recv_value["send_uid"] = uid;
	recv_value["recv_uid"] = recv_uid;
	recv_value["message_id"] = msg_id;
	recv_value["name"] = name;
	recv_value["thread_id"] = thread_id;
	recv_value["type"] = 2;
	recv_value["total_size"] = file_size;
	recv_session->Send(recv_value.toStyledString(), MSG_NOTIFY_DOWNLOAD_IMG_CHAT_RSP);
	rsp->set_error(ErrorCodes::SUCCESS);
	return Status::OK;
}

Status ResourceServiceImpl::NotifyChatVideoMsg(::grpc::ServerContext* context, const::message::GetChatVideoMsgReq* req, ::message::GetChatVideoMsgRsp* rsp){
	auto uid = req->uid();
	auto recv_uid = req->recv_uid();
	auto msg_id = req->msg_id();
	auto name = req->name();
	auto thread_id = req->thread_id();
	auto coverName = req->cover_name();
	auto duration = req->duration();

	auto session = UserMgr::GetInstance()->getUidSession(uid);
	if (session == nullptr) {
		std::cout << "NotifyChatImgMsg User session is nullptr" << std::endl;
		rsp->set_error(ErrorCodes::UidInvalid);
		return Status::OK;
	}

	Json::Value rtvalue;
	rtvalue["error"] = ErrorCodes::SUCCESS;
	rtvalue["send_uid"] = uid;
	rtvalue["recv_uid"] = recv_uid;
	rtvalue["message_id"] = msg_id;
	rtvalue["name"] = name;
	rtvalue["thread_id"] = thread_id;
	rtvalue["cover_name"] = coverName;
	rtvalue["duration"] = duration;
	std::cout << "Notify: " << rtvalue.toStyledString() << std::endl;
	session->Send(rtvalue.toStyledString(), MSG_NOTIFY_UPLOAD_VIDEO_CHAT_MSG_RSP);
	rsp->set_error(ErrorCodes::SUCCESS);

	auto recv_session = UserMgr::GetInstance()->getUidSession(recv_uid);
	if (recv_session == nullptr) {
		std::cout << "NotifyChatImgMsg Recv User session is nullptr" << std::endl;
		rsp->set_error(ErrorCodes::ERR_RPCFialed);
		return Status::OK;
	}

	Json::Value recv_value;
	recv_value["error"] = ErrorCodes::SUCCESS;
	recv_value["send_uid"] = uid;
	recv_value["recv_uid"] = recv_uid;
	recv_value["message_id"] = msg_id;
	recv_value["name"] = name;
	recv_value["thread_id"] = thread_id;
	recv_value["cover_name"] = coverName;
	recv_value["duration"] = duration;
	recv_session->Send(recv_value.toStyledString(), MSG_NOTIFY_DOWNLOAD_VIDEO_CHAT_MSG_RSP);
	rsp->set_error(ErrorCodes::SUCCESS);
	return Status::OK;
}
