#include "UserMgr.h"
#include <iostream>
#include "CSession.h"

UserMgr::~UserMgr(){
    _uid_to_session.clear();
}

std::shared_ptr<CSession> UserMgr::getUidSession(int uid){
    std::unique_lock<std::mutex> lock(_session_mtx);

    auto iter_find = _uid_to_session.find(uid);
    if (iter_find == _uid_to_session.end()) {
        return nullptr;
    }
    return _uid_to_session[uid];
}

void UserMgr::setUidSession(int uid, std::shared_ptr<CSession> session){
    std::unique_lock<std::mutex> lock(_session_mtx);
    _uid_to_session[uid] = session;
}

void UserMgr::RmvUidSession(int uid){
    {
        std::unique_lock<std::mutex> lock(_session_mtx);
        auto iter_find = _uid_to_session.find(uid);
        if (iter_find == _uid_to_session.end()) {
            return;
        }

        if (iter_find->second->getUid() != uid) {
            // 在其他地方登录
            return;
        }
        _uid_to_session.erase(uid);
    }
}

UserMgr::UserMgr()
{
}
