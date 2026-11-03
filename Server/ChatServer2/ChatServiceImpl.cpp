#include "ChatServiceImpl.h"
#include "UserMgr.h"
#include "Global.h"
#include "CSession.h"
#include "MysqlMgr.h"
#include "data.h"

ChatServiceImpl::ChatServiceImpl()
{
}

Status ChatServiceImpl::NotifyAddFriend(::grpc::ServerContext* context, const ::message::AddFriendReq* req, ::message::AddFriendRsp* rsp) {
	// 先获取发起好友申请用户的uid对应的session
	auto uid = req->fromuid();
	auto session = UserMgr::GetInstance()->getUidSession(uid);
	Defer defer([&rsp, &req]() {
		rsp->set_error(ErrorCodes::SUCCESS);
		rsp->set_fromuid(req->fromuid());
		rsp->set_touid(req->touid());
		});

	// 没找到
	if (session == nullptr) {
		return Status::OK;
	}

	Json::Value rtvalue;
	rtvalue["error"] = ErrorCodes::SUCCESS;
	rtvalue["fromuid"] = uid;
	rtvalue["touid"] = req->touid();
	rtvalue["name"] = req->name();
	rtvalue["desc"] = req->desc();
	rtvalue["nick"] = req->nick();
	rtvalue["icon"] = req->icon();
	rtvalue["sex"] = req->sex();
	session->Send(rtvalue.toStyledString(), MSG_APPLY_FRIEND_RSP);
	return Status::OK;
}

Status ChatServiceImpl::NotifyAuthFriend(::grpc::ServerContext* context, const ::message::AuthFriendReq* req, ::message::AuthFriendRsp* rsp) {
	auto touid = req->touid();
	auto session = UserMgr::GetInstance()->getUidSession(touid);

	Defer defer([this, &req, &rsp]() {
		rsp->set_error(ErrorCodes::SUCCESS);
		rsp->set_fromuid(req->fromuid());
		rsp->set_touid(req->touid());
		});
	if (session == nullptr) {
		return Status::OK;
	}

	Json::Value notify;
	notify["error"] = ErrorCodes::SUCCESS;
	notify["self_uid"] = req->touid();
	notify["friend_uid"] = req->fromuid();
	std::shared_ptr<UserInfo> info(new UserInfo);
	std::string key = USER_BASE_INFO + std::to_string(req->fromuid());
	GetBaseInfo(key, info);
	if (info) {
		notify["name"] = info->name;
		notify["nick"] = info->nick;
		notify["icon"] = info->icon;
		notify["sex"] = info->sex;
		auto& obj = req->textmsgs();
		notify["thread_id"] = obj.Get(0).thread_id();
		notify["message_id"] = obj.Get(0).message_id();
		notify["self_msg"] = obj.Get(0).msgcontent();
		notify["friend_msg"] = "我们已经是朋友了，快来开始聊天吧";
	}
	else {
		notify["error"] = ErrorCodes::UidInvalid;
	}

	session->Send(notify.toStyledString(), MSG_NOTIFY_FRIEND_RSP);
	return Status::OK;
}

Status ChatServiceImpl::NotifyTextChatMsg(::grpc::ServerContext* context, const ::message::TextChatMsgReq* req, ::message::TextChatMsgRsp* rsp) {
	auto friend_uid = req->touid();
	auto session = UserMgr::GetInstance()->getUidSession(friend_uid);
	if (session == nullptr) {
		return Status::OK;
	}

	Json::Value rtvalue, text_array;
	rtvalue["error"] = ErrorCodes::SUCCESS;
	rtvalue["friend_uid"] = req->fromuid();
	rtvalue["self_uid"] = friend_uid;

	for (const auto& obj : req->textmsgs()) {
		auto content = obj.msgcontent();
		auto id = obj.msgid();
		Json::Value element;
		element["msgid"] = id;
		element["content"] = content;
		text_array.append(element);
	}
	rtvalue["text_array"].append(text_array);

	session->Send(rtvalue.toStyledString(), MSG_NOTIFY_TEXT_CHAT_MSG_RSP);
	return Status::OK;
}

Status ChatServiceImpl::NotifyKickUser(::grpc::ServerContext* context, const::message::KickUserReq* req, ::message::KickUserRsp* rsp){
	auto uid = req->uid();
	auto session = UserMgr::GetInstance()->getUidSession(uid);
	if (session == nullptr) {
		return Status::OK;
	}

	Defer defer([&rsp, &req]() {
		rsp->set_error(ErrorCodes::SUCCESS);
		rsp->set_uid(req->uid());
		});

	Json::Value rtvalue;
	rtvalue["error"] = ErrorCodes::SUCCESS;
	rtvalue["uid"] = req->uid();
	session->Send(rtvalue.toStyledString(), MSG_NOTIFY_OFF_LINE);
	return Status::OK;
}
