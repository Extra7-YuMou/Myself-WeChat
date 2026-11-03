#pragma once
#include <fstream>
#include <string>
#include <map>
#include "Singleton.h"
#include <vector>
#include <filesystem>
#include <iostream>

struct SectionInfo {
	SectionInfo(){}
	SectionInfo(const SectionInfo& sec) {
		_section_data = sec._section_data;
	}

	SectionInfo& operator = (const SectionInfo & sec){
		if (&sec == this) {
			return *this;
		}

		_section_data = sec._section_data;
	}

	std::string operator[](const std::string& key) {
		if (_section_data.find(key) == _section_data.end()) {
			return "";
		}

		return _section_data[key];
	}

	~SectionInfo() {
		_section_data.clear();
	}

	std::map<std::string, std::string> _section_data;
};

// 读取配置文件类
class ConfigMgr : public Singleton<ConfigMgr> {
	friend class Singleton<ConfigMgr>;
public:
	~ConfigMgr();
	SectionInfo operator[](const std::string& key) {
		if (_config_map.find(key) == _config_map.end()) {
			return SectionInfo();
		}
		return _config_map[key];
	}

	std::string getSelfServerName();
	std::filesystem::path getOutputPath() {
		std::string staticDir = _config_map["Static"]["Path"];
		std::string outputDir = _config_map["Output"]["Path"];
		auto path = std::filesystem::current_path() / outputDir / staticDir;
		// 如果没有该文件夹则会自动创建
		std::filesystem::create_directories(path);
		std::cout << "path: " << path << std::endl;
		return path;
	}

	void printAll() const;
private:
	ConfigMgr();
	ConfigMgr(const ConfigMgr&) = delete;
	ConfigMgr& operator=(const ConfigMgr&) = delete;

	std::map<std::string, SectionInfo> _config_map;
	std::string first;
};

