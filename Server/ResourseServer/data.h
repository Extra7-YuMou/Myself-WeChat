#pragma once
#include <string>

struct UserInfo {
	UserInfo(const int& uid = 0, const std::string& name = "", const std::string& nick = "",
		const std::string& icon = "", const std::string& email = "", const int& sex = 0, const std::string& desc = "") :
		_uid(uid), _name(name), _nick(nick), _icon(icon), _email(email), _sex(sex), _desc(desc){ }

	int _uid;
	std::string _name;
	std::string _nick;
	std::string _icon;
	int _sex;
	std::string _desc;
	std::string _email;
};