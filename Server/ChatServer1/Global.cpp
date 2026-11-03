#include "Global.h"
#include "MysqlMgr.h"
#include "RedisMgr.h"
#include "data.h"

bool GetBaseInfo(const std::string& key, std::shared_ptr<UserInfo> ui) {
	std::string baseInfo;
	bool b_getInfo = RedisMgr::GetInstance()->Get(key, baseInfo);
	if (b_getInfo) {
		Json::Reader reader;
		Json::Value root;
		bool b_parse = reader.parse(baseInfo, root);
		if (b_parse) {
			ui->uid = root["uid"].asInt();
			ui->name = root["name"].asString();
			ui->email = root["email"].asString();
			ui->icon = root["icon"].asString();
			ui->desc = root["desc"].asString();
			ui->nick = root["nick"].asString();
			ui->sex = root["sex"].asInt();
		}
	}
	else {
		std::string uid_str = key.substr(std::string(USER_BASE_INFO).length());
		int uid = std::atoi(uid_str.c_str());
		ui = MysqlMgr::GetInstance()->GetUser(uid);

		Json::Value root;
		root["uid"] = ui->uid;
		root["name"] = ui->name;
		root["email"] = ui->email;
		root["nick"] = ui->nick;
		root["icon"] = ui->icon;
		root["desc"] = ui->desc;
		root["sex"] = ui->sex;
		RedisMgr::GetInstance()->Set(key, root.toStyledString());
	}

	return true;
}

bool GetUserByName(const std::string& key, std::shared_ptr<UserInfo> ui) {
	std::string userInfo;
	bool b_getUser = RedisMgr::GetInstance()->Get(key, userInfo);
	if (b_getUser) {
		Json::Value root;
		Json::Reader reader;
		reader.parse(userInfo, root);
		ui->uid = root["uid"].asInt();
		ui->name = root["name"].asString();
		ui->email = root["email"].asString();
		ui->nick = root["nick"].asString();
		ui->icon = root["icon"].asString();
		ui->sex = root["sex"].asInt();
		ui->desc = root["desc"].asString();
	}
	else {
		ui = MysqlMgr::GetInstance()->GetUserByName(key.substr(std::string(NAME_INFO).length()));
		Json::Value root;
		root["uid"] = ui->uid;
		root["name"] = ui->name;
		root["email"] = ui->email;
		root["nick"] = ui->nick;
		root["icon"] = ui->icon;
		root["desc"] = ui->desc;
		root["sex"] = ui->sex;
		RedisMgr::GetInstance()->Set(key, root.toStyledString());
	}
	return true;
}

bool GetUserByUid(const std::string& key, std::shared_ptr<UserInfo> ui) {
	std::string userInfo;
	bool b_getUser = RedisMgr::GetInstance()->Get(key, userInfo);
	if (b_getUser) {
		Json::Value root;
		Json::Reader reader;
		reader.parse(userInfo, root);
		ui->uid = root["uid"].asInt();
		ui->name = root["name"].asString();
		ui->email = root["email"].asString();
		ui->nick = root["nick"].asString();
		ui->icon = root["icon"].asString();
		ui->sex = root["sex"].asInt();
		ui->desc = root["desc"].asString();
	}
	else {
		auto uid_str = key.substr(std::string(UID_INFO).length());
		int uid = std::atoi(uid_str.c_str());
		ui = MysqlMgr::GetInstance()->GetUserByUid(uid);
		if (ui == nullptr) {
			return true;
		}
		Json::Value root;
		root["uid"] = ui->uid;
		root["name"] = ui->name;
		root["email"] = ui->email;
		root["nick"] = ui->nick;
		root["icon"] = ui->icon;
		root["desc"] = ui->desc;
		root["sex"] = ui->sex;
		RedisMgr::GetInstance()->Set(key, root.toStyledString());
	}
	return true;
}

std::string Gbk2utf8(const char* strGBK)
{
	int len = MultiByteToWideChar(CP_ACP, 0, strGBK, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_ACP, 0, strGBK, -1, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
	std::string strTemp = str;
	if (wstr) delete[] wstr;
	if (str) delete[] str;
	return strTemp;
}
