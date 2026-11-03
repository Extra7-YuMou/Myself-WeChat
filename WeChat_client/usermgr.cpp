#include "usermgr.h"

UserMgr::UserMgr() : _uid(0), _name(""), _token("")
{
    PATH_PREFIX = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

void UserMgr::setName(const QString &name){
    _name = name;
}

void UserMgr::setUid(const int& uid){
    _uid = uid;
}

void UserMgr::setToken(const QString& token){
    qDebug() << "存入token, token: " << token;
    _token = token;
}

void UserMgr::setNick(const QString &nick)
{
    _nick = nick;
}

void UserMgr::setIcon(const QString &icon)
{
    _icon = icon;
}

void UserMgr::setDesc(const QString &desc)
{
    _desc = desc;
}

void UserMgr::setSex(const int &sex)
{
    _sex = sex;
}

int UserMgr::getUid()
{
    return _uid;
}

QString UserMgr::getName()
{
    return _name;
}

QString UserMgr::getToken()
{
    return _token;
}

QString UserMgr::getNick()
{
    return _nick;
}

QString UserMgr::getIcon()
{
    return _icon;
}

QString UserMgr::getDesc()
{
    return _desc;
}

int UserMgr::GetSex()
{
    return _sex;
}

QString UserMgr::GetPathPrefix()
{
    return PATH_PREFIX;
}

QMap<int, std::shared_ptr<AddFriendApply> > &UserMgr::getFriendApplys()
{
    return _friend_applys;
}

std::vector<std::shared_ptr<ApplyInfo> > &UserMgr::getApplyList()
{
    return _apply_list;
}

QMap<int, QListWidgetItem *> &UserMgr::getChatItemAdded()
{
    return _chat_item_added;
}

void UserMgr::addChatThread(std::shared_ptr<ChatThreadInfo> info)
{
    if(!info){
        return;
    }

    auto iter_find = std::find(_chat_threads_info.begin(), _chat_threads_info.end(), info);
    if(iter_find != _chat_threads_info.end()){
        return;
    }

    _chat_threads_info.push_back(info);
}

QVector<std::shared_ptr<ChatThreadData> > &UserMgr::GetChatThreadList()
{
    return _chat_threads;
}

QMap<int, std::shared_ptr<UserInfo> > &UserMgr::GetFriendList()
{
    return _friend_list;
}

std::shared_ptr<UserInfo> UserMgr::GetSelfInfo()
{
    return std::make_shared<UserInfo>(_uid, _name, _nick, _icon, _sex, _desc);
}

void UserMgr::AddFileInfoByMd5(const QString &md5, std::shared_ptr<QFileInfo> fileInfo)
{
    _md5_files[md5] = fileInfo;
}

std::shared_ptr<QFileInfo> UserMgr::GetFileInfoByMd5(const QString &md5)
{
    auto iter_find = _md5_files.find(md5);
    if(iter_find == _md5_files.end()){
        return nullptr;
    }
    return iter_find.value();
}

bool UserMgr::IsDownLoad(const QString &name)
{
    std::lock_guard<std::mutex> lock(_mtx);
    auto iter_find = _name_to_download_info.find(PATH_PREFIX + name);
    if(iter_find == _name_to_download_info.end()){
        return false;
    }
    return true;
}

void UserMgr::AddDownLoadFile(const QString &name, std::shared_ptr<FileInfo> fileInfo, const QString& dir_name)
{
    std::lock_guard<std::mutex> lock(_mtx);
    auto targetPath = PATH_PREFIX + "/" + dir_name + "/" + fileInfo->_name;
    fileInfo->_name = targetPath;
    _name_to_download_info[PATH_PREFIX + name] = fileInfo;
}

std::shared_ptr<FileInfo> UserMgr::GetDownLoadFile(const QString &name)
{
    std::lock_guard<std::mutex> lock(_mtx);
    auto iter_find = _name_to_download_info.find(PATH_PREFIX + name);
    if(iter_find == _name_to_download_info.end()){
        return nullptr;
    }
    return iter_find.value();
}

void UserMgr::RmvDownLoadFile(const QString &name)
{
    std::lock_guard<std::mutex> lock(_mtx);
    auto iter_find = _name_to_download_info.find(PATH_PREFIX + name);
    if(iter_find != _name_to_download_info.end()){
        _name_to_download_info.erase(iter_find);
    }
}

void UserMgr::AddResetLabel(const QString &name, QLabel *label)
{
    std::lock_guard<std::mutex> lock(_mtx);
    _name_to_reset_label[PATH_PREFIX + name] = label;
}

QLabel *UserMgr::GetResetLabel(const QString &name)
{
    std::lock_guard<std::mutex> lock(_mtx);
    auto iter_find = _name_to_reset_label.find(PATH_PREFIX + name);
    if(iter_find == _name_to_reset_label.end()){
        return nullptr;
    }
    return iter_find.value();
}

void UserMgr::RmvResetLabel(const QString &name)
{
    std::lock_guard<std::mutex> lock(_mtx);
    auto iter_find = _name_to_reset_label.find(PATH_PREFIX + name);
    if(iter_find == _name_to_reset_label.end()){
        return;
    }
    _name_to_reset_label.erase(iter_find);
}
