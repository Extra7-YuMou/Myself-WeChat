#include "ConfigMgr.h"

ConfigMgr::~ConfigMgr()
{
}

void ConfigMgr::printAll() const{
	// 输出所有的section和key-value对  
	for (const auto& section_entry : _config_map) {
		const std::string& section_name = section_entry.first;
		SectionInfo section_config = section_entry.second;
		std::cout << "[" << section_name << "]" << std::endl;
		for (const auto& key_value_pair : section_config._section_data) {
			std::cout << key_value_pair.first << "=" << key_value_pair.second << std::endl;
		}
	}
}

ConfigMgr::ConfigMgr() {
	std::ifstream in("config.ini");
	if (!in.is_open()) {
		std::cout << "config.ini 配置文件读取失败，请重试!" << std::endl;
		in.close();
		return;
	}

	std::cout << "config.ini 配置文件读取成功!" << std::endl;
	std::string line;
	SectionInfo section;
	while (std::getline(in, line)) {
		// 跳过注释
		if (line[0] == '#') {
			continue;
		}

		// End结束
		if (line == "End") {
			_config_map[first] = section;
		}

		// 哪个服务上的，如[GateServer]
		if (line[0] == '[' && line[line.length() - 1] == ']') {
			if (!section._section_data.empty()) {
				_config_map[first] = section;
				section = SectionInfo();
			}
			first = line.substr(1, line.length() - 2);
			continue;
		}

		// 读取服务里的键值对
		std::size_t pos = line.find('=');
		if (pos != std::string::npos) {
			std::string key = line.substr(0, pos - 1);
			std::string value = line.substr(pos + 2);
			section._section_data[key] = value;
		}
	}
}